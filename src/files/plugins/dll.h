#pragma once

#include "../files.h"

void BigEndian() {
    bigEndian = true;
}
void LittleEndian() {
    bigEndian = false;
}

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
    std::memcpy(&var, &data[file_pos], sizeof(var));
    if (bigEndian == true) var = toBigEndian(var); else var = toLittleEndian(var);
    file_pos += sizeof(var);
}

std::string parse_array_char(int size, const char* data) {
    std::string buffer = "";
    if (size > 0) {
        for (int i = 0; i < size; i++) {
            if (data[file_pos] != '\0') {
                buffer += data[file_pos];
            }
            file_pos++;
        }
    } else {
        for (int i = 0; data[file_pos] != '\0'; i++) {
            buffer += data[file_pos];
            file_pos++;
        }
        file_pos++;
    }
    return buffer;
}