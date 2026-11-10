#ifndef GAME_CLOCK_H
#define GAME_CLOCK_H

class GameClock {
private:
    int day;
    double elapsed_seconds;
    double day_duration_seconds;

    int elapsed_game_minutes() const;

public:
    explicit GameClock(int day, double duration);

    void update(float elapsed);

    int get_day() const;
    int get_hour() const;
    int get_minute() const;

    bool is_finished() const;
    void start_next_day();
};

#endif