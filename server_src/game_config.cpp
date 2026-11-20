
#include "game_config.h"

#include <cstdint>
#include <stdexcept>
#include <toml++/toml.hpp>


// usamos static para que la funcion solo sea visible dentro de este cpp
// data: el contenido del archivo TOML que ya fue parseado.
// section: la sección donde queremos buscar el valor.
// key: el nombre del parámetro que queremos leer.
// devuelve un int con el valor leido
static int read_positive_int(const toml::table& data,
                      const std::string& section,
                      const std::string& key) {
    //
    auto value = data[section][key].value<int>();

    if (!value || *value <= 0) {
        throw std::runtime_error(
            "Valor invalido en configuracion: " + section + "." + key);
    }
    // value devuelve un optional int, por lo tanto para devolver in int puro
    // usamos el *, asi devolvemos el valor es si del optional
    return *value;
}

static std::map<std::string, int> read_price_table(
    const toml::table& data,
    const std::string& section) {

    const auto* table = data[section].as_table();

    if (!table) {
        throw std::runtime_error("Seccion de precios inexistente: " + section);
    }

    std::map<std::string, int> prices;

    for (const auto& [key, node] : *table) {
        //value devuelve un optional con * se obtiene el int
        auto price = node.value<int>();

        if (!price || *price < 0) {
            throw std::runtime_error("Precio invalido: " + section + "." + std::string(key.str()));
        }

        prices.emplace(std::string(key.str()), *price);
    }

    return prices;
}

GameConfig GameConfig::load(const std::string& path) {
    const toml::table data = toml::parse_file(path);

    return GameConfig{
        read_positive_int(data, "jugador", "vida_inicial"),
        read_positive_int(data, "jugador", "energia_inicial"),
        read_positive_int(data, "jugador", "dano_espada"),
        read_positive_int(data, "tiempo", "duracion_dia_segundos"),
        read_price_table(data, "precios_compra"),
        read_price_table(data, "precios_venta")
    };
}