#ifndef UTILS_H
#define UTILS_H


#include <string>

enum class Pet { NONE = 00, CAT = 01, DOG = 02 };

enum class Op { JOIN = 1, LIST = 2, CREATE = 3 };

struct GameInfo {
    int current;
    int required;
    std::string name;
};

struct JoinInfo {
    Pet pet;
    std::string game_name;
};

struct CreateInfo {
    Pet pet;
    int required;
    std::string game_name;
};

struct GameResult {
    bool success;
    bool full;
};

enum class Command { LIST, JOIN, CREATE, FIN };


#endif
