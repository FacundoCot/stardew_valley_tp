# Servidor del TP Stardew Valley — Documentación de las clases

Este documento explica la parte del **servidor** que se implementó para la demo por terminal: un mapa, partidas con jugadores, validación de movimientos y el envío del estado a los clientes. Está pensado para que alguien que no participó en el desarrollo pueda leerlo de arriba hacia abajo y entender qué hace cada pieza, por qué existe y cómo se conecta con las demás.

**Qué cubre este documento**

- Las clases **creadas**: `Map`, `Game`, `ClientRegistry`, `GameManager`.
- Las clases **modificadas**: `ClientHandler`, `Acceptor`, `Server` y el `main` del servidor.
- Una **simulación completa** de una sesión de juego, paso a paso.

## 1. Visión general

| Problema | Quién lo resuelve |
|---|---|
| Aceptar muchas conexiones a la vez | `Acceptor` + `ClientHandler` |
| Convertir los bytes de cada cliente en pedidos y ordenarlos | `ClientReceiver` + la cola `actions` (preexistentes) |
| Aplicar las reglas del juego en un único lugar | `GameManager` + `Game` + `Map` (**nuevos**) |
| Responderle a un cliente concreto (o a todos los de una partida) | `ClientRegistry` + `ClientSender` |

### 1.3 Hilos y quién toca qué

| Hilo | Cantidad | Qué hace | Qué estructuras toca |
|---|---|---|---|
| principal (`Server::run`) | 1 | Lee `stdin` esperando `q` | Coordina el apagado |
| `Acceptor` | 1 | `accept()` y alta/baja de clientes | `clients`, **`ClientRegistry`** (`add`/`remove`) |
| `ClientReceiver` | 1 por cliente | Lee del socket y encola pedidos | Solo escribe en `actions` |
| `ClientSender` | 1 por cliente | Saca de su cola de salida y escribe al socket | Solo lee su `outgoing` |
| `gameloop` | 1 | Ejecuta los pedidos | **`GameManager`, `Game`, `Map`**, **`ClientRegistry`** (`send`) |


## 2. Clases creadas

### 2.1 Map

**Archivos:** `common_src/map.h`, `common_src/map.cpp`

#### Qué es

Una grilla rectangular de caracteres que describe el mundo. Cada celda es una pared (`#`), un piso (`.`) o un punto de aparición de jugadores (`S`).

#### Para qué sirve y qué problema resuelve

Responde dos preguntas: *"¿puede un jugador pisar esta celda?"* y *"¿dónde aparecen los jugadores?"*. Sin `Map`, las reglas de movimiento tendrían las dimensiones y las paredes escritas a mano dentro de `Game`.

Está en `common_src/` (y no en `server_src/`) porque el cliente también lo va a necesitar para **dibujar** el mapa. Así ambos lados comparten una única definición.

#### Atributos

| Atributo | Tipo | Significado |
|---|---|---|
| `width` | `int` | Cantidad de columnas (eje x) |
| `height` | `int` | Cantidad de filas (eje y) |
| `grid` | `std::vector<std::string>` | Una `string` por fila. Se accede como `grid[y][x]` (primero la fila, después la columna) |

Constantes internas (en un `namespace` anónimo de `map.cpp`, no visibles desde afuera): `WALL = '#'`, `FLOOR = '.'`, `SPAWN = 'S'`.

#### Funciones

##### `Map(int map_width, int map_height)` — constructor

- **Parámetros:** ancho y alto del mapa, en celdas.
- **Devuelve:** nada (construye el objeto).
- **Precondición:** `map_width >= 3` y `map_height >= 3`. Con menos no existe un interior donde poner el spawn. No hay validación: no lanza excepción, simplemente el mapa queda **sin spawn** 
- **Postcondición:** `grid` tiene `height` filas de `width` caracteres. El perímetro es `#`, hay una única `S` en `(width/2, height/2)` y el resto es `.`.
- **Cómo funciona por dentro:**
  1. Inicializa `width` y `height` en la lista de inicialización.
  2. Calcula la posición del spawn con división entera: `spawn_col = width / 2`, `spawn_row = height / 2`. Para 20×20 da `(10, 10)`.
  3. Recorre todas las celdas con dos `for` anidados (`row` por fuera, `col` por dentro) y arma cada fila en un `std::string line`. Para cada celda evalúa **en este orden**:
     - Si `col == 0`, `row == 0`, `col == width-1` o `row == height-1` → `#` (borde).
     - Si no, si `(col, row)` coincide con el spawn → `S`.
     - Si no → `.`.
  4. Al terminar cada fila hace `grid.push_back(line)`.
  - El orden de los `if` importa: el borde se evalúa primero, por lo que el spawn nunca puede caer sobre una pared.

##### `bool is_walkable(int x, int y) const`

- **Parámetros:** coordenadas `x` (columna) e `y` (fila) de la celda a consultar.
- **Devuelve:** `true` si el jugador puede estar en esa celda; `false` si está fuera del mapa o es una pared.
- **Precondición:** ninguna; acepta cualquier valor entero, incluso negativos o fuera de rango.
- **Postcondición:** no modifica el mapa (`const`).
- **Cómo funciona:** primero chequea los límites (`x < 0 || y < 0 || x >= width || y >= height` → `false`). Recién después lee `grid[y][x]` y devuelve `true` si **no** es `#`. Ese orden evita leer fuera del vector. Como la `S` no es pared, **el spawn es caminable**.
- **Para qué se usa:** `Game::add_player` y `Game::move_player` lo consultan antes de aceptar cualquier posición.

##### `const std::vector<std::string>& rows() const`

- **Parámetros:** ninguno.
- **Devuelve:** una referencia de solo lectura a las filas de la grilla.
- **Postcondición:** no copia ni modifica nada. La referencia es válida mientras el `Map` exista.
- **Para qué se usa:** para que el **cliente** dibuje el mapa.
- **Sobre los dos `const`:** el de adelante impide modificar el vector devuelto; el de atrás declara que la función no modifica el `Map`, lo que permite llamarla sobre un `Map` constante.

##### `std::vector<std::pair<int, int>> spawns() const`

- **Parámetros:** ninguno.
- **Devuelve:** una lista de pares `(x, y)`, uno por cada celda marcada con `S`. Está vacía si el mapa no tiene ninguna.
- **Postcondición:** devuelve una **copia**; modificarla no afecta al mapa.
- **Cómo funciona:** recorre la grilla (filas de arriba hacia abajo, cada fila de izquierda a derecha) y agrega `(x, y)` cuando encuentra `S`. Hoy el constructor coloca un solo spawn, así que la lista tiene un elemento. La función ya soporta varios por si el editor de mapas los define.
- **Orden de los componentes del par:** `first` es `x`, `second` es `y`.

#### Resumen del código interno

`Map` es un contenedor de datos con una sola regla: *una celda es caminable si está dentro de los límites y no es `#`*. Todo lo demás (generar el borde, ubicar el spawn) ocurre una sola vez en el constructor. Es copiable (contiene solo un `vector`), y eso se aprovecha: `GameManager` guarda un mapa base y le da **una copia** a cada partida.

#### Notas

- Tamaños pequeños: `Map(3,3)` tiene su spawn en `(1,1)`; `Map(4,4)` en `(2,2)`; `Map(2,2)` **no tiene spawn** (la celda `(1,1)` es borde); `Map(0,0)` queda sin filas y `is_walkable` siempre devuelve `false`.
- Un mapa sin spawn hace que `Game::add_player_auto` devuelva `false`, y por lo tanto que crear o unirse a una partida responda `ERROR`.

---

### 2.2 Game

**Archivos:** `server_src/game.h`, `server_src/game.cpp`

#### Qué es

**Una partida.** Guarda un mapa propio y la posición de cada jugador conectado, y aplica las reglas de movimiento.

#### Para qué sirve y qué problema resuelve

Concentra las reglas del juego en una clase que **no sabe nada de redes, sockets ni mensajes**. Esa separación tiene dos ventajas: se puede probar con un `main` de consola sin levantar el servidor, y si cambian las reglas no hay que tocar el código de red.

#### Atributos

| Atributo | Tipo | Significado |
|---|---|---|
| `map` | `Map` | Copia del mapa de esta partida |
| `players` | `std::map<int, Player>` | Jugadores conectados: `client_id` → posición |

`Player` es una estructura simple con dos enteros, `x` e `y`. Se usa `std::map` (y no `unordered_map`) para que al recorrer los jugadores el orden sea siempre el mismo (por `client_id` creciente).

#### Funciones

##### `explicit Game(Map map_param)` — constructor

- **Parámetros:** el mapa que usará la partida.
- **Postcondición:** la partida existe sin jugadores.
- **Cómo funciona:** mueve el mapa recibido a su atributo con `std::move`, para no copiarlo dos veces (el parámetro ya es una copia). `explicit` impide conversiones implícitas accidentales de un `Map` a un `Game`.

##### `bool add_player(int client_id, int x, int y)`

- **Parámetros:** el id del cliente y la celda donde aparecerá.
- **Devuelve:** `true` si el jugador se agregó; `false` si no.
- **Precondición:** ninguna.
- **Postcondición:** si devuelve `true`, `players[client_id]` vale `(x, y)`. Si devuelve `false`, **nada cambió**.
- **Cuándo devuelve `false`:**
  1. El `client_id` ya está en la partida (un cliente no puede tener dos jugadores).
  2. La celda no es caminable (pared o fuera del mapa).
- **Cómo funciona:** usa `players.count(client_id)` para detectar el id repetido, consulta `map.is_walkable(x, y)` y, si todo está bien, hace `players[client_id] = Player{x, y}`.

##### `bool add_player_auto(int client_id)`

- **Parámetros:** el id del cliente.
- **Devuelve:** `true` si el jugador quedó ubicado en el spawn; `false` si el mapa no tiene spawn o si el id ya existe.
- **Postcondición:** si devuelve `true`, el jugador está en el primer spawn del mapa.
- **Cómo funciona:** pide `map.spawns()`; si la lista está vacía devuelve `false`; si no, toma el primero (`spawns.front()`), lo desarma con `auto [x, y]` y delega en `add_player(client_id, x, y)`. No repite validaciones: las hace `add_player`.
- **Diferencia con `add_player`:** `add_player` recibe la posición exacta; `add_player_auto` **decide** la posición (el spawn). Es la que usa `GameManager`, porque el cliente no elige dónde aparece.

##### `bool move_player(int client_id, Direction direction)`

- **Parámetros:** el id del jugador y la dirección (`UP`, `DOWN`, `LEFT`, `RIGHT`).
- **Devuelve:** `true` si el jugador se movió una celda; `false` si no se movió.
- **Precondición:** ninguna (si el jugador no existe, devuelve `false`).
- **Postcondición:** si devuelve `true`, la posición cambió exactamente una celda. Si devuelve `false`, **la posición no cambió**.
- **Cómo funciona:**
  1. Busca al jugador con `players.find(client_id)`. Si `find` devuelve `end()`, no existe → `false`.
  2. Copia su posición actual en `new_x` y `new_y`. **Son variables de trabajo**: la posición real todavía no se toca.
  3. Un `switch` ajusta esas copias: `UP` resta 1 a `y`, `DOWN` suma 1 a `y`, `LEFT` resta 1 a `x`, `RIGHT` suma 1 a `x`. (El eje `y` crece hacia abajo, igual que las filas del mapa.)
  4. Si `map.is_walkable(new_x, new_y)` es `false` → `false`, sin cambios.
  5. Recién ahí escribe `new_x` y `new_y` en el jugador y devuelve `true`.
- **Por qué usar copias de trabajo:** garantiza que un movimiento rechazado nunca deje al jugador en una celda inválida.
- **Por qué el `switch` no tiene `default`:** cubre todos los valores de `Direction`. El `ServerProtocol` ya descarta cualquier dirección fuera de 1–4 antes de crear el pedido.
- **Sobre `player_iterator->second.x`:** el iterador de un `std::map` apunta a un par. `first` es la clave (`client_id`) y `second` es el valor (el `Player`). Por eso `->second.x` es "la coordenada x del jugador".

##### `bool remove_player(int client_id)`

- **Devuelve:** `true` si el jugador existía y fue eliminado; `false` si no existía.
- **Cómo funciona:** `players.erase(client_id)` devuelve cuántos elementos borró (0 o 1); se compara con `> 0`.

##### `bool is_empty() const`

- **Devuelve:** `true` si no queda ningún jugador. `GameManager` lo usa para borrar la partida cuando se va el último.

##### `std::vector<int> player_ids() const`

- **Devuelve:** los `client_id` de todos los jugadores, en orden creciente.
- **Para qué se usa:** para saber a quiénes enviarles el estado.
- **Cómo funciona:** recorre `players` con `for (const auto& [id, player] : players)` (el `[id, player]` desarma cada par en dos nombres) y junta los ids. Hace `reserve` de antemano para evitar reasignaciones de memoria.

##### `std::vector<PlayerState> snapshot() const`

- **Devuelve:** una foto del estado: un `PlayerState` (`id`, `x`, `y`) por jugador, ordenados por id.
- **Para qué se usa:** es el contenido del mensaje `GAME_STATE`.
- **Cómo funciona:** recorre los jugadores y convierte cada valor con `static_cast` a los tipos de tamaño fijo del protocolo (`uint32_t` para el id, `uint16_t` para las coordenadas).
- **Por qué hay conversión:** dentro de `Game` todo es `int`, porque las validaciones necesitan números negativos (por ejemplo, `x - 1` con `x = 0` da `-1`). Los tipos de tamaño fijo se usan **solo en el borde con la red**. La conversión a `uint16_t` es segura porque `add_player` solo acepta celdas caminables, que siempre son no negativas.

#### Resumen del código interno

`Game` es un diccionario `client_id → posición` más un mapa de solo lectura. La única regla de negocio es *"una posición es válida si el mapa la considera caminable"*. Todas las funciones que cambian algo devuelven un `bool` y, ante un `false`, dejan el estado intacto. Eso permite que `GameManager` decida qué mensaje mandar sin tener que deshacer nada.

#### Notas

- **No es thread-safe, y no lo necesita:** solo la usa el hilo `gameloop`.
---

### 2.3 ClientRegistry

**Archivos:** `server_src/client_registry.h`, `server_src/client_registry.cpp`

#### Qué es

Una "agenda" que asocia cada `client_id` con **la cola de salida** de ese cliente.

#### Para qué sirve y qué problema resuelve

Cada cliente tiene su propia cola de salida (`outgoing`), pero esa cola vive dentro de su `ClientHandler`, que pertenece al `Acceptor`. El `GameManager` no tiene acceso a ella.

Cuando Ana se mueve, el `GameManager` tiene que avisarle a Ana **y a Bruno**. Solo conoce los ids `1` y `2`. Sin el registro no habría forma de llegar a la cola de Bruno. Con el registro basta con:

```cpp
registry.send(2, mensaje);   // busca la cola del cliente 2 y le deja el mensaje
```

Es el puente entre *"quién es el jugador"* (el id que usa `Game`) y *"por dónde le mando el mensaje"* (la cola que consume su `ClientSender`).

#### Atributos

| Atributo | Tipo | Significado |
|---|---|---|
| `mtx` | `std::mutex` | Protege `queues` |
| `queues` | `std::map<int, Queue<ServerMessage>*>` | `client_id` → puntero a su cola de salida |

Guarda **punteros** (no copias): la cola real pertenece al `ClientHandler` y el registro solo apunta a ella.

#### Funciones

##### `void add(int client_id, Queue<ServerMessage>* outgoing)`

- **Parámetros:** el id del cliente y un puntero a su cola de salida.
- **Precondición:** `outgoing` es un puntero válido y la cola seguirá existiendo hasta que se llame a `remove(client_id)`.
- **Postcondición:** el cliente queda registrado; a partir de ahora `send(client_id, ...)` llega a esa cola. Si el id ya existía, se sobrescribe.
- **Quién la llama:** el `Acceptor`, al aceptar un cliente.
- **Cómo funciona:** toma el mutex con `std::lock_guard` y asigna `queues[client_id] = outgoing`.

##### `void remove(int client_id)`

- **Parámetros:** el id del cliente a quitar.
- **Precondición:** debe llamarse **antes** de destruir el `ClientHandler` correspondiente.
- **Postcondición:** el cliente deja de estar registrado. Cuando `remove` vuelve, **ninguna llamada a `send` está usando ya ese puntero**. Si el id no existía, no hace nada.
- **Quién la llama:** el `Acceptor`, en `reap()` y `clean()`.
- **Por qué importa el orden:** si el handler se destruyera primero, el registro quedaría con un puntero a memoria liberada y el próximo `send` escribiría en ella (comportamiento indefinido).
- **Cómo funciona:** toma el mutex y hace `queues.erase(client_id)`.

##### `void send(int client_id, const ServerMessage& message)`

- **Parámetros:** el id del destinatario y el mensaje a enviarle.
- **Devuelve:** nada. **No lanza excepciones.**
- **Postcondición:** si el cliente existe y su cola está abierta, el mensaje queda **copiado** en su cola, y su `ClientSender` lo escribirá en el socket. Si el cliente no existe, o su cola ya está cerrada, no pasa nada.
- **Quién la llama:** el `GameManager`.
- **Cómo funciona:**
  1. Toma el mutex.
  2. Busca el id con `queues.find`. Si no está, retorna (un cliente que se acaba de ir es un caso normal, no un error).
  3. Hace `push` del mensaje en la cola.
  4. Si el `push` lanza `ClosedQueue` (el cliente se está desconectando), lo captura y lo ignora.
- **Por qué mantiene el mutex durante el `push`:** es lo que garantiza la postcondición de `remove`. Mientras un `send` usa el puntero tiene el mutex tomado, así que `remove` tiene que esperar a que termine antes de borrar. El `push` no se bloquea en la práctica porque las colas de salida no tienen límite de tamaño.

#### Resumen del código interno

Un `std::map` protegido por un único mutex. Las tres operaciones toman el mutex con `std::lock_guard` (RAII: se libera al salir de la función aunque haya una excepción).

#### Por qué necesita mutex

Lo usan **dos hilos a la vez**: el del `Acceptor` (`add` / `remove` al conectar y desconectar clientes) y el `gameloop` (`send`). Sin mutex, un `send` podría recorrer el `std::map` mientras el `Acceptor` lo modifica y corromperlo.

---

### 2.4 GameManager

**Archivos:** `server_src/game_manager.h`, `server_src/game_manager.cpp`

#### Qué es

El **coordinador de las partidas**. Implementa la interfaz `GameActionHandler`: es el objeto contra el que se ejecutan los pedidos de los clientes.

#### Para qué sirve y qué problema resuelve

Entre los pedidos que llegan (crear, unirse, mover, desconectar) y las reglas del juego (`Game`) falta alguien que:

- sepa **qué partidas existen** y cuál es el código de cada una;
- sepa **en qué partida está cada cliente**;
- decida **qué mensaje enviar y a quién** según el resultado.

Ese es `GameManager`. Es el único que habla a la vez con `Game` (reglas) y con `ClientRegistry` (envío).

#### Atributos

| Atributo | Tipo | Significado |
|---|---|---|
| `registry` | `ClientRegistry&` | Referencia al registro (lo posee `Server`) |
| `world_map` | `Map` | Mapa base; cada partida recibe una **copia** |
| `games` | `std::map<std::string, Game>` | Código de partida → partida |
| `client_game` | `std::map<int, std::string>` | `client_id` → código de la partida donde está |
| `rng` | `std::mt19937` | Generador de números aleatorios para los códigos |

El mapa `client_game` existe porque los pedidos `MOVE` y `DISCONNECT` solo traen el `client_id`; con él se averigua en qué partida buscar.

#### Funciones privadas (auxiliares)

##### `std::string generate_code()`

- **Devuelve:** un código de 4 letras mayúsculas (`A–Z`) que **no está en uso** por ninguna partida existente.
- **Cómo funciona:** un `do { ... } while` que arma 4 letras tomando posiciones aleatorias de `"ABCDEFGHIJKLMNOPQRSTUVWXYZ"` (con `std::uniform_int_distribution`) y repite mientras `games.count(code) > 0`. Hay 26⁴ = 456.976 códigos posibles. El generador `rng` se siembra con `std::random_device`.

##### `void send_error(int client_id, const std::string& text)`

- **Qué hace:** arma un `ServerMessage` de tipo `ERROR` con el texto y se lo envía a **un** cliente vía `registry.send`.

##### `void send_state(const Game& game)`

- **Qué hace:** arma un `ServerMessage` de tipo `GAME_STATE` con `game.snapshot()` y se lo envía a **todos** los jugadores de esa partida (`game.player_ids()`), llamando `registry.send` por cada uno. Es el *broadcast* del servidor.
- **Por qué el mismo mensaje para todos:** cada cliente recibe la foto completa; así todos ven lo mismo (el servidor es la única fuente de verdad).

#### Funciones públicas (las del `GameActionHandler`)

##### `GameManager(ClientRegistry& registry, Map world_map)` — constructor

- **Parámetros:** el registro y el mapa base.
- **Cómo funciona:** guarda la referencia al registro, mueve el mapa a `world_map` y siembra `rng`.

##### `void create_game(int client_id, const std::string& game_name, const std::string& player_name)`

- **Parámetros:** quién pide, el nombre de la partida y el nombre del jugador. **Hoy `game_name` y `player_name` se reciben pero no se usan** (están marcados con `(void)` para evitar el warning de variable sin usar); se guardarán cuando `Game` tenga nombres.
- **Devuelve:** nada; la respuesta va por mensajes.
- **Precondición:** ninguna.
- **Postcondición (éxito):** existe una partida nueva con un código único; el cliente está en ella, en el spawn; recibió `GAME_CREATED` y después `GAME_STATE`.
- **Postcondición (fallo):** no queda ninguna partida a medias; el cliente recibió un `ERROR`.
- **Cómo funciona:**
  1. Si `client_game` ya contiene al cliente → `ERROR "Ya estas en una partida"` y termina. (Un cliente no puede estar en dos partidas a la vez.)
  2. Genera un código y crea la partida: `games.emplace(code, Game(world_map))`.
  3. Intenta ubicar al jugador con `add_player_auto`. Si falla (el mapa no tiene spawn) → borra la partida recién creada (`games.erase(code)`) y envía `ERROR "No hay lugar en el mapa"`.
  4. Registra `client_game[client_id] = code`.
  5. Envía `GAME_CREATED` al creador, con el código y su `player_id` (que es su `client_id`).
  6. Llama a `send_state(game)`.
- **Orden de los mensajes que recibe el creador:** primero `GAME_CREATED`, después `GAME_STATE`.

##### `void join_game(int client_id, const std::string& game_code, const std::string& player_name)`

- **Parámetros:** quién pide, el código de la partida y el nombre del jugador (este último, sin uso por ahora).
- **Postcondición (éxito):** el cliente está en la partida, en el spawn. Él recibe `GAME_JOINED` y después `GAME_STATE`; los demás jugadores reciben `GAME_STATE`.
- **Postcondición (fallo):** el estado no cambia; el cliente recibe un `ERROR`.
- **Cómo funciona:**
  1. Si ya está en una partida → `ERROR "Ya estas en una partida"`.
  2. Busca el código en `games`. Si no existe → `ERROR "No existe una partida con ese codigo"`. (El código distingue mayúsculas.)
  3. Intenta `add_player_auto`. Si falla → `ERROR "No hay lugar en la partida"`.
  4. Registra `client_game[client_id] = game_code`.
  5. Envía `GAME_JOINED` al que entró.
  6. Llama a `send_state(game)`, que llega a **todos**, incluido el nuevo.

##### `void move_player(int client_id, Direction direction)`

- **Parámetros:** quién se mueve y hacia dónde.
- **Postcondición:** si el movimiento fue válido, **todos** los jugadores de la partida reciben el nuevo `GAME_STATE`. Si no fue válido (pared, jugador sin partida), **no se envía nada** y el estado no cambia.
- **Cómo funciona:** busca la partida del cliente en `client_game` (si no está → vuelve en silencio); busca la partida en `games`; llama a `Game::move_player`. Solo si devuelve `true` ejecuta `send_state`.
- **Por qué no responde con error:** para un mapa de demo basta con ignorar el movimiento inválido. Si se quisiera feedback, se agregaría un `send_error` en la rama `false`.

##### `void disconnect_client(int client_id)`

- **Parámetros:** el cliente que se desconectó.
- **Postcondición:** el cliente ya no figura en ninguna partida. Si la partida quedó vacía, se elimina (y no se envía nada, porque no hay a quién). Si quedan jugadores, reciben el `GAME_STATE` actualizado.
- **Quién lo dispara:** el `ClientReceiver` del cliente, al detectar que el socket se cerró, genera un `DisconnectAction`.
- **Cómo funciona:**
  1. Si el cliente no estaba en ninguna partida (por ejemplo, se conectó y se fue sin crear ni unirse) → no hace nada.
  2. Guarda el código y borra la entrada de `client_game`.
  3. Busca la partida; llama a `game.remove_player`.
  4. Si `game.is_empty()` → `games.erase(...)`; si no → `send_state(game)`.
- **Detalle técnico:** `games.erase(game_it)` invalida la referencia `game`. Por eso el código hace `return` inmediatamente después de borrar y no vuelve a usar `game`.

#### Resumen del código interno

Dos diccionarios (`games` y `client_game`) y cuatro métodos que siguen siempre el mismo patrón: **validar → delegar en `Game` → enviar mensajes según el resultado**. `GameManager` no contiene reglas del juego (eso es `Game`) ni toca sockets (eso es `ClientRegistry`): solo orquesta.

#### Por qué no necesita mutex

Todos sus métodos se invocan desde el hilo `gameloop`, que procesa **un pedido por vez**. La cola `actions` es la que serializa los pedidos de todos los clientes.

---

## 3. Clases modificadas

### 3.1 ClientHandler

**Archivos:** `server_src/client_handler.h`, `server_src/client_handler.cpp`

#### Qué es

Agrupa **todo lo que pertenece a un cliente conectado**: su socket, su cola de salida y los dos hilos (`ClientReceiver` y `ClientSender`).

#### Qué se modificó

1. Se agregó el atributo `int client_id`.
2. Se agregó `int id() const`.
3. Se agregó `Queue<ServerMessage>& outgoing_queue()`.

**Para qué:** el `Acceptor` necesita el id y la cola para registrar al cliente en el `ClientRegistry`. Antes `outgoing` era privado y no había forma de alcanzarlo.

#### Atributos (en orden de declaración)

| Atributo | Significado |
|---|---|
| `skt` | Socket del cliente |
| `outgoing` | Cola de mensajes que el servidor le enviará a este cliente |
| `client_id` | Id asignado por el `Acceptor` |
| `receiver` | Hilo que lee el socket y encola pedidos |
| `sender` | Hilo que lee `outgoing` y escribe al socket |
| `stopped` | Evita hacer el cierre dos veces |

**El orden de declaración importa.** C++ construye los miembros en el orden en que están declarados, no en el de la lista de inicialización. `receiver` y `sender` reciben referencias a `skt` y `outgoing`, así que estos tienen que estar construidos antes. Lo mismo vale para `client_id`, que se usa para construir el receiver.

#### Funciones

##### `ClientHandler(Socket&& skt, Queue<std::shared_ptr<ClientAction>>& actions, int client_id)` — constructor

- **Parámetros:** el socket recién aceptado (se mueve adentro), la cola `actions` compartida y el id asignado.
- **Postcondición:** el handler está armado pero **los hilos todavía no corren**.

##### `void start()`

- **Qué hace:** arranca el hilo del receiver y el del sender.

##### `void stop()`

- **Postcondición:** los hilos reciben la orden de detenerse y se desbloquean. Es **idempotente**: llamarla dos veces no hace nada la segunda vez (`stopped`).
- **Cómo funciona:** marca `stopped`; llama `receiver.stop()` y `sender.stop()` (solo bajan una bandera); **cierra `outgoing`** (así el `pop()` del sender, que estaba esperando, lanza `ClosedQueue` y el hilo termina); y hace `skt.shutdown(2)` (así el `recv` del receiver, que estaba bloqueado, retorna). Las excepciones del `shutdown` se ignoran porque el socket puede estar ya cerrado.
- **Por qué hace falta cerrar la cola y el socket:** ambos hilos están bloqueados en llamadas que no miran la bandera. Solo cerrar la cola o el socket los despierta.

##### `bool is_finished()`

- **Devuelve:** `true` si el receiver **o** el sender terminaron (`!receiver.is_alive() || !sender.is_alive()`).
- **Para qué se usa:** el `Acceptor` lo consulta en `reap()` para detectar clientes caídos.

##### `int id() const`

- **Devuelve:** el `client_id`. **(Nueva)**

##### `Queue<ServerMessage>& outgoing_queue()`

- **Devuelve:** una referencia a la cola de salida. **(Nueva)** El `Acceptor` toma su dirección para registrarla.

##### `~ClientHandler()`

- **Cómo funciona:** llama a `stop()` y luego hace `join` de ambos hilos. Así no queda ningún hilo corriendo con referencias a un objeto que ya no existe (RAII).

---

### 3.2 Acceptor

**Archivos:** `server_src/acceptor.h`, `server_src/acceptor.cpp`

#### Qué es

El hilo que **acepta conexiones nuevas** y mantiene la lista de clientes conectados.

#### Qué se modificó

1. El constructor recibe un `ClientRegistry&` y lo guarda.
2. En `run()`, cada cliente nuevo se **registra** con `registry.add(...)`.
3. En `reap()` y `clean()`, cada cliente se **quita del registro** con `registry.remove(...)` antes de destruirlo.

#### Atributos

| Atributo | Significado |
|---|---|
| `skt` | Socket servidor (escucha) |
| `actions` | Cola de pedidos, que se pasa a cada `ClientHandler` |
| `registry` | Registro donde se anotan las colas de salida |
| `next_client_id` | Contador de ids, arranca en 1 |
| `clients` | `std::vector<std::unique_ptr<ClientHandler>>`: los handlers vivos, de los que el `Acceptor` es dueño |

#### Funciones

##### `Acceptor(Socket&& skt, Queue<...>& actions, ClientRegistry& registry)` — constructor

- **Cómo funciona:** mueve el socket y guarda las dos referencias.

##### `void run() override` (corre en su propio hilo)

- **Cómo funciona:**
  1. Mientras `should_keep_running()`: `skt.accept()` (**bloquea** hasta que llega una conexión).
  2. Si en el medio se pidió parar, sale del bucle.
  3. Llama `reap()` para limpiar clientes caídos.
  4. Crea el handler con `std::make_unique<ClientHandler>(..., next_client_id++)`. Cada conexión recibe un id nuevo; **los ids nunca se reutilizan**.
  5. **Registra la cola:** `registry.add(new_handler->id(), &new_handler->outgoing_queue())`.
  6. Mueve el handler al vector `clients` (el `Acceptor` pasa a ser su dueño).
  7. Llama `clients.back()->start()` para arrancar los hilos del cliente.
  8. Si ocurre una excepción mientras se debía seguir corriendo, la guarda con `std::current_exception()`; si fue porque se pidió parar (el `accept` falla al cerrar el socket), se considera un cierre normal y se ignora.
  9. Al salir, llama `clean()` y, si había una excepción guardada, la relanza.
- **Por qué `registry.add` va antes de `start()`:** cuando los hilos del cliente empiezan, el cliente ya es alcanzable desde el registro.
- **Por qué el handler se mueve al vector antes de `start()`:** para que ya haya un dueño del objeto antes de que existan hilos que lo usen.

##### `void stop() override`

- **Qué hace:** baja la bandera (`Thread::stop()`) y hace `shutdown` y `close` del socket de escucha. Esto **despierta** al `accept()` bloqueado, que lanza una excepción y deja salir al hilo.

##### `void reap()` (privada)

- **Qué hace:** recorre `clients`; por cada handler con `is_finished() == true`, hace `registry.remove(id)` y **después** `clients.erase(it)` (cuya destrucción hace `join` de los hilos).
- **Por qué `remove` primero:** `erase` destruye el handler y su cola. Si el registro siguiera apuntando a ella, quedaría un puntero colgante.
- **Cuándo corre:** solo cuando entra un cliente nuevo (ver limitaciones).

##### `void clean()` (privada)

- **Qué hace:** al apagar, por cada cliente hace `registry.remove(id)` y `client->stop()`; después `clients.clear()` destruye todos los handlers (cada destructor hace `join` de sus hilos).

#### Regla de oro de esta clase

Siempre: **primero `registry.remove`, después destruir el handler.**

---

### 3.3 Server

**Archivos:** `server_src/server.h`, `server_src/server.cpp`

#### Qué es

El objeto que **arma todas las piezas** del servidor y controla su ciclo de vida.

#### Qué se modificó

1. Se agregaron los atributos `registry`, `manager` y `gameloop`.
2. Se agregó el método `game_loop()`.
3. `run()` ahora arranca el gameloop y apaga todo en orden.
4. Se eliminó la copia del objeto (`= delete`).

#### Atributos (en orden de declaración)

| Atributo | Significado |
|---|---|
| `actions` | Cola de pedidos de todos los clientes |
| `registry` | `ClientRegistry` compartido |
| `manager` | `GameManager` (usa `registry`) |
| `acceptor` | Acepta conexiones (usa `actions` y `registry`) |
| `gameloop` | `std::thread` que ejecuta los pedidos |

El orden es una restricción real: `manager` y `acceptor` reciben una **referencia** a `registry` en su constructor, así que `registry` tiene que estar construido antes.

#### Funciones

##### `explicit Server(Socket&& skt)` — constructor

- **Cómo funciona:** inicializa los miembros en orden. **Acá se define el mapa del juego:** `manager(registry, Map(20, 20))`. Cambiar el tamaño del mapa es cambiar esa línea.

##### `void game_loop()` (privada; corre en el hilo `gameloop`)

- **Qué hace:** un bucle infinito: `actions.pop()` (espera un pedido) y `action->execute(manager)`.
- **Manejo de errores:** si ejecutar un pedido lanza una excepción, se imprime en `stderr` y el bucle **sigue**: un pedido malo no debe tirar abajo el servidor.
- **Condición de salida:** cuando `run()` cierra `actions`, `pop()` lanza `ClosedQueue` y el bucle termina.

##### `void run()`

- **Cómo funciona:**
  1. Arranca el `Acceptor` y el hilo `gameloop`.
  2. Lee `stdin` caracter por caracter hasta encontrar `q`.
  3. **Apagado ordenado:**
     1. `acceptor.stop()` y `acceptor.join()`: deja de aceptar clientes; al terminar, `clean()` ya cerró y destruyó todos los handlers.
     2. `actions.close()`: desbloquea el `pop()` del gameloop.
     3. `gameloop.join()`: espera a que el hilo termine.
- **Por qué ese orden:** primero se cortan las **fuentes** de pedidos (clientes), después se cierra la cola y por último se espera al consumidor. Así el gameloop no se queda sin cola mientras los clientes todavía están enviando.
- **Cumple el enunciado:** cuando se cierra el servidor, los sockets de los clientes se cierran (ellos ven el fin de la conexión). Cuando se cierra un cliente, el servidor sigue funcionando.

#### Notas

- Si `stdin` se cierra (EOF), el bucle de lectura termina y el servidor se apaga. Por eso hay que dejarlo corriendo en una terminal interactiva 
- El `gameloop` es un `std::thread` directo, no la clase `Thread` del repo.

---

### 3.4 main (server_main.cpp)

#### Qué hace

1. Verifica que se haya pasado exactamente un argumento (el puerto); si no, imprime el uso y devuelve 1.
2. Crea el `Server` con `Socket(servername)`, que abre el socket en modo servidor en ese puerto.
3. Llama a `srv.run()`, que bloquea hasta que se escribe `q`.
4. Captura cualquier excepción, la imprime y devuelve 1; si todo va bien devuelve 0.

#### Uso

```bash
./game_server 18080
```
---

## 4. Simulación completa de una sesión

Se simulan cuatro clientes sobre el servidor real. Los mensajes que se muestran son la **salida real** de la ejecución. El código de la partida es aleatorio: en esta corrida salió `JQOZ`.

**Mapa de la partida** (20×20; la `S` es el spawn):

```
####################
#..................#
#..................#
        ...
#.........S........#      ← fila 10, columna 10
        ...
#..................#
####################
```

### Paso 0 — Los clientes se conectan

**Qué pasa:** cada cliente abre una conexión TCP.

- **Código que corre:** `Acceptor::run` → `accept()` → `reap()` → crea el `ClientHandler` con el siguiente id → `registry.add(id, &cola)` → `start()`.
- **Resultado:** Ana es el cliente `1`, Bruno el `2`, Carlos el `3`, Dani el `4`. El id depende del orden de conexión, **no** de si el cliente entra a una partida.
- **Por qué:** desde este momento el cliente tiene dos hilos (receiver y sender) y su cola está en el registro, así que el `GameManager` podrá escribirle.

### Paso 1 — Ana crea la partida

**Ana envía:** `CREATE_GAME`, nombre de partida `"granja"`, nombre de jugador `"Ana"`.

**Camino interno:**

1. `ClientReceiver 1` lee los bytes → `ServerProtocol::recv_action` arma un `CreateGameAction` → `set_client_id(1)` → `actions.push`.
2. El `gameloop` hace `actions.pop()` y llama `execute(manager)`, que termina en `GameManager::create_game(1, ...)`.
3. `create_game`: Ana no está en ninguna partida → `generate_code()` produce `JQOZ` → `games.emplace("JQOZ", Game(mapa))` → `add_player_auto(1)` la ubica en el spawn `(10,10)` → `client_game[1] = "JQOZ"`.
4. El manager envía dos mensajes con `registry.send(1, ...)`: `GAME_CREATED` y `send_state` (`GAME_STATE`).
5. Cada mensaje queda en la cola de salida de Ana; `ClientSender 1` lo saca y lo escribe en el socket.

**Ana recibe:**

```
Ana <- ('GAME_CREATED', 'JQOZ', 1)
Ana <- ('GAME_STATE', [(1, 10, 10)])
```

**Por qué:** Ana necesita el código para compartirlo con sus amigos, y su id para reconocerse a sí misma dentro de `GAME_STATE`.

### Paso 2 — Bruno se une con el código

**Bruno envía:** `JOIN_GAME`, código `"JQOZ"`, nombre `"Bruno"`.

**Camino interno:** igual que antes hasta `GameManager::join_game(2, "JQOZ", ...)`: Bruno no está en una partida → el código existe → `add_player_auto(2)` lo ubica en el spawn → `client_game[2] = "JQOZ"` → `GAME_JOINED` solo para Bruno → `send_state` a **todos** los de la partida (`player_ids()` devuelve `[1, 2]`).

**Bruno recibe:**

```
Bruno <- ('GAME_JOINED', 'JQOZ', 2)
Bruno <- ('GAME_STATE', [(1, 10, 10), (2, 10, 10)])
```

**Ana recibe** (sin haber pedido nada):

```
Ana <- ('GAME_STATE', [(1, 10, 10), (2, 10, 10)])
```

**Por qué:** la partida cambió, así que **todos** reciben la foto actualizada. Los dos están en `(10,10)` porque hay un solo spawn

### Paso 3 — Ana se mueve hacia abajo

**Ana envía:** `MOVE`, dirección `2` (abajo).

**Camino interno:** `MoveAction::execute` → `GameManager::move_player(1, DOWN)` → `client_game[1]` da `"JQOZ"` → `Game::move_player`: calcula `(10,11)`, `is_walkable` da `true`, la posición se actualiza, devuelve `true` → como fue válido, `send_state`.

**Ana y Bruno reciben lo mismo:**

```
Ana   <- ('GAME_STATE', [(1, 10, 11), (2, 10, 10)])
Bruno <- ('GAME_STATE', [(1, 10, 11), (2, 10, 10)])
```

**Por qué:** el servidor es la verdad. El cliente de Ana no se movió por su cuenta: esperó este mensaje para dibujar el cambio.

### Paso 4 — Bruno baja a la celda de Ana

**Bruno envía:** `MOVE`, dirección `2`.

**Resultado:** el movimiento es válido aunque Ana esté en esa celda, porque no se controla la ocupación.

```
Ana   <- ('GAME_STATE', [(1, 10, 11), (2, 10, 11)])
Bruno <- ('GAME_STATE', [(1, 10, 11), (2, 10, 11)])
```

### Paso 5 — Ana choca contra la pared

**Ana envía** 9 movimientos a la izquierda; todos son válidos y cada uno genera un `GAME_STATE` para los dos. Después de esos 9, Ana está en `x = 1`. Su décimo `MOVE` a la izquierda apuntaría a `(0, 11)`, que es borde.

**Camino interno:** `Game::move_player` calcula `(0,11)`, `is_walkable` da `false` y devuelve `false`. `GameManager::move_player` ve `false` y **no envía nada**.

**Resultado:** ni Ana ni Bruno reciben ningún mensaje, y la posición de Ana sigue siendo `(1,11)`.

### Paso 6 — Casos de error

| Acción | Quién | Respuesta |
|---|---|---|
| `JOIN_GAME` con código `"ZZZZ"` | Carlos (cliente 3) | `ERROR "No existe una partida con ese codigo"` |
| `CREATE_GAME` estando ya en una partida | Ana | `ERROR "Ya estas en una partida"` |
| `MOVE` sin estar en ninguna partida | Carlos | Nada (se ignora en silencio) |

```
Carlos <- ('ERROR', 'No existe una partida con ese codigo')
Ana    <- ('ERROR', 'Ya estas en una partida')
```

### Paso 7 — Bruno se desconecta

**Qué pasa:** Bruno cierra su conexión.

**Camino interno:**

1. El `recv` de `ClientReceiver 2` falla → el receiver crea un `DisconnectAction` con `client_id = 2` y lo encola.
2. El `gameloop` lo ejecuta → `GameManager::disconnect_client(2)`: borra `client_game[2]`, `Game::remove_player(2)`, la partida **no** queda vacía → `send_state`.

**Ana recibe:**

```
Ana <- ('GAME_STATE', [(1, 1, 11)])
```

**Por qué:** los demás jugadores deben dejar de ver a Bruno. El cierre de la conexión es el único aviso que el servidor tiene de que Bruno se fue.

### Paso 8 — Ana se desconecta y la partida desaparece ----> cambiar a que tiene que seguir guardada

**Camino interno:** `disconnect_client(1)` → `remove_player` → `is_empty()` da `true` → `games.erase`. No se envía nada, porque no queda nadie.

**Dani intenta unirse con `"JQOZ"`:**

```
Dani <- ('ERROR', 'No existe una partida con ese codigo')
```

**Por qué:** una partida sin jugadores se borra. (Ver limitaciones: el enunciado pide persistencia, que todavía no existe.)

### Paso 9 — El servidor sigue funcionando

Dani crea una partida nueva con otro código y aparece de nuevo en el spawn:

```
Dani <- ('GAME_CREATED', 'SEUT', 4)
Dani <- ('GAME_STATE', [(4, 10, 10)])
```

### Paso 10 — Apagado

El operador escribe `q` en la terminal del servidor.

1. `Server::run` sale del bucle de lectura.
2. `acceptor.stop()` despierta el `accept()`; `acceptor.join()` espera a que `clean()` quite del registro, detenga y destruya a todos los clientes. Los clientes conectados ven el cierre de la conexión.
3. `actions.close()` desbloquea el `gameloop`, que termina.
4. `gameloop.join()`; `main` devuelve 0.


### Resumen de quién hace qué en cada paso

| Momento | Hilo / clase que actúa |
|---|---|
| Aceptar la conexión y registrar la cola | `Acceptor` |
| Leer los bytes y armar el pedido | `ClientReceiver` + `ServerProtocol` |
| Ordenar los pedidos de todos los clientes | cola `actions` |
| Decidir qué hacer con el pedido | `GameManager` (en el hilo `gameloop`) |
| Validar la posición | `Game` + `Map` |
| Elegir a qué cola mandar la respuesta | `ClientRegistry` |
| Escribir los bytes en el socket | `ClientSender` + `ServerProtocol` |

---

## 5. Compilar, ejecutar y probar

### Compilar (dentro del contenedor Docker del curso)

```bash
cd /mnt
g++ -std=gnu++2a -Wall -Wextra -pthread -I. \
    common_src/*.cpp server_src/*.cpp server_main.cpp \
    -o game_server
```

### Ejecutar

```bash
./game_server 18080
```

Dejar la terminal abierta: el servidor se apaga si `stdin` se cierra. Para detenerlo, escribir `q` y Enter.

## 6. Decisiones de diseño

| Decisión | Motivo |
|---|---|
| `Game` no conoce sockets ni mensajes | Se puede probar sin red y las reglas quedan separadas del transporte. |
| Un solo hilo (`gameloop`) ejecuta todos los pedidos | `GameManager`, `Game` y `Map` no necesitan mutex; la cola `actions` ordena los pedidos. |
| `ClientRegistry` con mutex | Es la única estructura compartida por dos hilos (`Acceptor` y `gameloop`). |
| `int` dentro de `Map` y `Game`; tipos de tamaño fijo solo en `snapshot()` | Las validaciones necesitan negativos; los tamaños exactos solo importan en el formato del mensaje. |
| El servidor decide dónde aparece cada jugador (`add_player_auto`) | El cliente es "tonto": no elige posición. |
| Un solo spawn | `Map::spawns()` ya soporta varios spawns. |
| Al volver a entrar, el jugador aparece en el spawn | El enunciado no define dónde reaparece; se eligió lo más simple y se puede revisar con el corrector. |
| `client_id` nuevo en cada conexión, sin reutilizar | Evita confundir a un cliente nuevo con uno ya desconectado. |
| El `Acceptor` quita del registro **antes** de destruir el handler | El registro guarda punteros; si no, quedarían colgantes. |
| Un movimiento inválido no genera mensaje | Alcanza para la demo; el cliente simplemente no ve cambios. |

---