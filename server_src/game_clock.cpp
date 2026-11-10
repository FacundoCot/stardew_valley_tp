#include "game_clock.h"

GameClock::GameClock(int day, double duration):
        day(day), 
        elapsed_seconds(0.0),
        day_duration_seconds(duration) {}

bool GameClock::is_finished() const {
    return elapsed_seconds >= day_duration_seconds;
}

void GameClock::update(float elapsed) {
    if (is_finished()) {
        return;
    }

    elapsed_seconds += elapsed;

    if (elapsed_seconds >= day_duration_seconds) {
        elapsed_seconds = day_duration_seconds;
    }
}

// 1200 es por 20 x 60, que son los minutos totales que hay desde las 6am hasta las 2am
// esto nos sirve para calcular los minutos virtuales del juego. 
// elapsed_seconds es el tiempo transcurrido en segundos, day_duration_seconds es la duración de un día en segundos
// Por lo tanto, elapsed_seconds / day_duration_seconds nos da la fracción del día que ha pasado
// Multiplicando por 1200 obtenemos los minutos transcurridos en el juego
int GameClock::elapsed_game_minutes() const {
    return elapsed_seconds * 1200 / day_duration_seconds;
}

// aumenta 1 el dia y vuelve elapsed a 0
void GameClock::start_next_day() {
    day++;
    elapsed_seconds = 0.0;
}

// el juego arranca a las 6 am, por eso se suma 6 a los minutos transcurridos
int GameClock::get_hour() const {
    return (6 + elapsed_game_minutes() / 60) % 24;
}

//devuelve los minutos transcurridos de la hora en la que estamos
int GameClock::get_minute() const {
    return elapsed_game_minutes() % 60;
}

int GameClock::get_day() const {
    return day;
}