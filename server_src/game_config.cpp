
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

static int read_crop_int(const toml::table& data,
                  const std::string& crop,
                  const std::string& key,
                  bool allow_zero = false) {

    auto value = data["cultivos"][crop][key].value<int>();

    if (!value || *value < 0 || (!allow_zero && *value == 0)) {
        throw std::runtime_error(
            "Valor invalido: cultivos." + crop + "." + key);
    }

    return *value;
}

GameConfig GameConfig::load(const std::string& path) {
    const toml::table data = toml::parse_file(path);

    GameConfig config{};

    config.initial_health =
        read_positive_int(data, "jugador", "vida_inicial");

    config.initial_energy =
        read_positive_int(data, "jugador", "energia_inicial");

    config.sword_damage =
        read_positive_int(data, "jugador", "dano_espada");

    config.day_duration_seconds =
        read_positive_int(data, "tiempo", "duracion_dia_segundos");

    config.buy_prices = read_price_table(data, "precios_compra");
    config.sell_prices = read_price_table(data, "precios_venta");

    config.crop_waterings = {
        {ItemType::STRAWBERRY,
         read_crop_int(data, "frutilla", "riegos_por_dia")},

        {ItemType::ONION,
         read_crop_int(data, "cebolla", "riegos_por_dia")},

        {ItemType::POTATO,
         read_crop_int(data, "papa", "riegos_por_dia")},

        {ItemType::PUMPKIN,
         read_crop_int(data, "calabaza", "riegos_por_dia")}
    };

    config.crop_growth_days = {
        {ItemType::STRAWBERRY,
         read_crop_int(data, "frutilla", "dias_crecimiento")},

        {ItemType::ONION,
         read_crop_int(data, "cebolla", "dias_crecimiento")},

        {ItemType::POTATO,
         read_crop_int(data, "papa", "dias_crecimiento")},

        {ItemType::PUMPKIN,
         read_crop_int(data, "calabaza", "dias_crecimiento")}
    };

    config.crop_regrowth_days = {
        {ItemType::STRAWBERRY,
         read_crop_int(data, "frutilla", "dias_rebrote", true)},

        {ItemType::ONION,
         read_crop_int(data, "cebolla", "dias_rebrote", true)},

        {ItemType::POTATO,
         read_crop_int(data, "papa", "dias_rebrote", true)},

        {ItemType::PUMPKIN,
         read_crop_int(data, "calabaza", "dias_rebrote", true)}
    };

    config.crop_missed_water_tolerance = {
        {ItemType::STRAWBERRY,
         read_crop_int(data, "frutilla",
                       "riegos_omitidos_permitidos", true)},

        {ItemType::ONION,
         read_crop_int(data, "cebolla",
                       "riegos_omitidos_permitidos", true)},

        {ItemType::POTATO,
         read_crop_int(data, "papa",
                       "riegos_omitidos_permitidos", true)},

        {ItemType::PUMPKIN,
         read_crop_int(data, "calabaza",
                       "riegos_omitidos_permitidos", true)}
    };

    config.till_energy_cost =
        read_positive_int(data, "energia_acciones", "preparar_tierra");

    config.plant_energy_cost =
        read_positive_int(data, "energia_acciones", "plantar");

    config.water_energy_cost =
        read_positive_int(data, "energia_acciones", "regar");

    config.harvest_energy_cost =
        read_positive_int(data, "energia_acciones", "cosechar");

    config.quality_watering_minutes_per_point =
        read_positive_int(data, "calidad_cultivos", "minutos_riego_por_punto");

    config.quality_missed_watering_penalty =
        read_positive_int(data, "calidad_cultivos", "penalizacion_riego_omitido");

    config.quality_harvest_minutes_per_point =
        read_positive_int(data, "calidad_cultivos", "minutos_cosecha_por_punto");

    config.quality_luck_min_pct =
        read_positive_int(data, "calidad_cultivos", "suerte_min_pct");

    config.quality_luck_max_pct =
        read_positive_int(data, "calidad_cultivos", "suerte_max_pct");

    config.quality_good_threshold =
        read_positive_int(data, "calidad_cultivos", "umbral_bueno");

    config.quality_excellent_threshold =
        read_positive_int(data, "calidad_cultivos", "umbral_excelente");

    config.quality_incredible_threshold =
        read_positive_int(data, "calidad_cultivos", "umbral_increible");

    config.quality_perfect_threshold =
        read_positive_int(data, "calidad_cultivos", "umbral_perfecta");

    return config;
}