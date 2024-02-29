#pragma once

#include "./files.h"

#define DefineInt(type, var) \
    type var; \
    parse_array_int(var, data)

plugin_metadata create_metadata(std::vector<const char*>& v_games, std::vector<const char*>& v_paths) {
    plugin_metadata metadata;
    metadata.games = v_games.data();
    metadata.games_count = v_games.size();
    metadata.paths = v_paths.data();
    metadata.paths_count = v_paths.size();
    return metadata;
}

template <std::integral T>
void parse_array_int(T& var, const char* data) {
    std::memcpy(&var, &data[Pos], sizeof(var));
    if (bigEndian == true) var = toBigEndian(var); else var = toLittleEndian(var);
    Pos += sizeof(var);
}

std::string parse_array_char(int size, const char* data) {
    std::string buffer = "";
    if (size > 0) {
        for (int i = 0; i < size; i++) {
            if (data[Pos] != '\0') {
                buffer += data[Pos];
            }
            Pos++;
        }
    } else {
        for (int i = 0; data[Pos] != '\0'; i++) {
            buffer += data[Pos];
            Pos++;
        }
        Pos++;
    }
    return buffer;
}