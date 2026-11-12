
#include "save_disc.h"

#include <fstream>
#include <stdexcept>

#include <nlohmann/json.hpp>

//definimos estas macros para que nlohmann::json pueda serializar y deserializar nuestras estructuras de datos
//asi nos ahorramos tener que escribir funciones de serializacion y deserializacion para cada estructura
//y casi no tenemos que tocar codigo para los jason 
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ItemSaveData, type, quality, quantity)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PlayerSaveData, name, x, y, health, coins, inventory)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GameSaveData, game_code, game_name, owner_name, multiplayer, day, players)

SaveDisc::SaveDisc(const std::filesystem::path& saves_directory):
        saves_directory(saves_directory) {
    //si no existe el directorio con el path de saves, lo crea
    std::filesystem::create_directories(saves_directory);
}

void SaveDisc::save(const GameSaveData& data) const {
    //ofstream crea un archivo con el nombre del game_code y la extension .json en el directorio de saves
    //se reescribe el archivo si ya existe
    // se puede hacer / porque el operador / de std::filesystem::path concatena correctamente los paths
    std::ofstream file(saves_directory / (data.game_code + ".json"));

    if (!file) {
        throw std::runtime_error("No se pudo guardar la partida");
    }
    // convierte el objeto GameSaveData a json gracias a las macros 
    // NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE y lo guarda en el archivo
    nlohmann::json content = data;
    file << content;
}

GameSaveData SaveDisc::load(const std::string& game_code) const {
    //ifstream abre un archivo con el nombre del game_code y la extension .json en el directorio de saves
    //se usa para leer el archivo, en cambio ofstream se usa para escribir en el archivo
    std::ifstream file(saves_directory / (game_code + ".json"));

    if (!file) {
        throw std::runtime_error("No se pudo cargar la partida");
    }
    
    //creamos un objeto json y leemos el contenido del archivo en el objeto json
    nlohmann::json content;
    file >> content;

    //convierte el objeto json a GameSaveData gracias a las macros NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE
    return content.get<GameSaveData>();
}

std::vector<std::string> SaveDisc::list_saves() const {
    std::vector<std::string> codes;

    for (const auto& entry : std::filesystem::directory_iterator(saves_directory)) {
        //si el archivo es un archivo regular y tiene extension .json, 
        //agregamos el nombre del archivo sin extension al vector de codes
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            // con .path() obtenemos el path completo del archivo, 
            // con .stem() obtenemos el nombre del archivo sin extension y
            // con .string() lo convertimos a string
            codes.push_back(entry.path().stem().string());
        }
    }

    return codes;
}
