#include "enemy_config.h"

#include <stdexcept>
#include <toml++/toml.hpp>

namespace {
int positive_int(const toml::table& data, const std::string& section,
                 const std::string& key) {
    auto value = data[section][key].value<int>();
    if (!value || *value <= 0) {
        throw std::runtime_error("Valor invalido en configuracion: " + section + "." + key);
    }
    return *value;
}

float positive_float(const toml::table& data, const std::string& section,
                     const std::string& key) {
    auto value = data[section][key].value<double>();
    if (!value || *value <= 0.0) {
        throw std::runtime_error("Valor invalido en configuracion: " + section + "." + key);
    }
    return static_cast<float>(*value);
}

float nested_positive_float(const toml::table& data, const std::string& parent,
                            const std::string& section, const std::string& key) {
    auto value = data[parent][section][key].value<double>();
    if (!value || *value <= 0.0) {
        throw std::runtime_error("Valor invalido en configuracion: " + parent +
                                 "." + section + "." + key);
    }
    return static_cast<float>(*value);
}

std::vector<EnemyDropRule> drops(const toml::table& data,
                                 const std::string& section) {
    std::vector<EnemyDropRule> result;
    const auto* array = data[section]["drops"].as_array();
    if (!array) {
        throw std::runtime_error("Valor invalido en configuracion: " + section + ".drops");
    }
    for (const toml::node& node : *array) {
        const auto* table = node.as_table();
        if (!table) {
            throw std::runtime_error("Valor invalido en configuracion: " + section + ".drops");
        }
        auto item = (*table)["item"].value<std::string>();
        auto min = (*table)["cantidad_min"].value<int>();
        auto max = (*table)["cantidad_max"].value<int>();
        auto chance = (*table)["probabilidad"].value<double>();
        if (!item || (*item != "oro" && *item != "plata") || !min || *min < 1 ||
            !max || *max < *min || !chance || *chance < 0.0 || *chance > 1.0) {
            throw std::runtime_error("Valor invalido en configuracion: " + section + ".drops");
        }
        result.push_back({*item == "oro" ? ItemType::GOLD : ItemType::SILVER,
                          Quality::BASE, *min, *max, static_cast<float>(*chance)});
    }
    return result;
}

EnemyTypeConfig enemy_type(const toml::table& data, const std::string& section) {
    EnemyTypeConfig result;
    result.max_health = positive_int(data, section, "vida");
    result.damage = positive_int(data, section, "dano");
    result.speed = positive_float(data, section, "velocidad");
    result.attack_cooldown = positive_float(data, section, "cooldown_ataque");
    result.attack_range = positive_float(data, section, "rango_ataque");
    result.drops = drops(data, section);
    return result;
}

int bounded_group(const toml::table& data, const std::string& key) {
    const int value = positive_int(data, "orco", key);
    if (value < 3 || value > 6) {
        throw std::runtime_error(
            "Valor invalido en configuracion: orco." + key +
            " debe estar entre 3 y 6");
    }
    return value;
}
}

EnemyConfig EnemyConfig::load(const std::string& path) {
    const toml::table data = toml::parse_file(path);
    EnemyConfig result;
    result.skeleton = enemy_type(data, "esqueleto");
    result.skeleton_revive_seconds = positive_float(data, "esqueleto", "tiempo_revivir");
    result.orc = enemy_type(data, "orco");
    result.orc_group_min = bounded_group(data, "grupo_min");
    result.orc_group_max = bounded_group(data, "grupo_max");
    if (result.orc_group_max < result.orc_group_min) {
        throw std::runtime_error("Valor invalido en configuracion: orco.grupo_max");
    }
    result.female_vampire = enemy_type(data, "vampira");
    result.male_vampire = enemy_type(data, "vampiro");
    result.blood_incident_radius = nested_positive_float(data, "vampiro", "sangre", "radio_incidente");
    result.blood_revive_seconds = nested_positive_float(data, "vampiro", "sangre", "tiempo_revivir");
    result.frenzy_multiplier = positive_float(data, "frenesi", "multiplicador");
    result.skull.shot_cooldown = positive_float(data, "calavera", "cooldown_disparo");
    result.skull.shot_range = positive_float(data, "calavera", "alcance_disparo");
    result.skull.blue_flame_damage = data["calavera"]["llama_azul"]["dano"].value<int>().value_or(0);
    result.skull.blue_flame_speed = nested_positive_float(data, "calavera", "llama_azul", "velocidad");
    result.skull.green_flame_damage = data["calavera"]["llama_verde"]["dano"].value<int>().value_or(0);
    result.skull.green_flame_speed = nested_positive_float(data, "calavera", "llama_verde", "velocidad");
    if (result.skull.blue_flame_damage <= 0) {
        throw std::runtime_error("Valor invalido en configuracion: calavera.llama_azul.dano");
    }
    if (result.skull.green_flame_damage <= 0) {
        throw std::runtime_error("Valor invalido en configuracion: calavera.llama_verde.dano");
    }
    result.skeletons_min = positive_int(data, "spawn", "esqueletos_min");
    result.skeletons_max = positive_int(data, "spawn", "esqueletos_max");
    result.vampire_pairs_min = positive_int(data, "spawn", "parejas_vampiros_min");
    result.vampire_pairs_max = positive_int(data, "spawn", "parejas_vampiros_max");
    result.skulls_per_level = positive_int(data, "spawn", "calaveras_por_nivel");
    if (result.skeletons_max < result.skeletons_min ||
        result.vampire_pairs_max < result.vampire_pairs_min ||
        result.vampire_pairs_max > 3) {
        throw std::runtime_error("Valor invalido en configuracion: spawn");
    }
    if (result.skulls_per_level > 1) {
        throw std::runtime_error(
            "Valor invalido en configuracion: spawn.calaveras_por_nivel "
            "no puede superar 1");
    }
    return result;
}
