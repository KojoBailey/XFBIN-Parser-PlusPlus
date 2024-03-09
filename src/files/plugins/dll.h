#pragma once

#include <regex>
#include <nlohmann/json.hpp>

using json = nlohmann::ordered_json;

// Types
using uint8 = std::uint8_t;     // unsigned integer 8-bit (1-byte)
using uint16 = std::uint16_t;   // unsigned integer 16-bit (2-byte)
using uint32 = std::uint32_t;   // unsigned integer 32-bit (3-byte)
using uint64 = std::uint64_t;   // unsigned integer 64-bit (4-byte)
using int8 = std::int8_t;       // signed integer 8-bit (1-byte)
using int16 = std::int16_t;     // signed integer 16-bit (2-byte)
using int32 = std::int32_t;     // signed integer 32-bit (4-byte)
using int64 = std::int64_t;     // signed integer 64-bit (8-byte)
using string = std::string;

#define str std::to_string

// Is the current system in big endian?
constexpr bool IS_BIG_ENDIAN = std::endian::native == std::endian::big;

// Convert to different endianness.
template <std::integral T> // Convert integer to big endian.
T toBigEndian(T value) {
    return (!IS_BIG_ENDIAN) 
        ? std::byteswap(value)
        : value;
}
template <std::integral T> // Convert integer to little endian.
T toLittleEndian(T value) {
    return (IS_BIG_ENDIAN) 
        ? std::byteswap(value)
        : value;
}

struct plugin_metadata {
    const char** games;
	size_t games_count;
    const char** paths;
	size_t paths_count;
};

#define EXPORT extern "C" __declspec(dllexport)
EXPORT plugin_metadata __stdcall get_plugin_metadata();
EXPORT const char* __stdcall unpack(const char* data);
EXPORT void __stdcall clean_up(const char** pointers, int size);

// Use regex to modify the original paths for different versions.
void Regex_Add_Paths(std::vector<std::string>& v_paths, std::vector<std::string>& regex_paths, std::vector<std::string>& supported_versions) {
    std::regex regex_pattern("XXX");
    for (const std::string& version : supported_versions) {
        for (const std::string& regex_path : regex_paths) {
            v_paths.push_back(std::regex_replace(regex_path, regex_pattern, version));
        }
    }
}

plugin_metadata create_metadata(std::vector<std::string>& v_games, std::vector<std::string>& v_paths) {
    plugin_metadata metadata;
    metadata.games_count = v_games.size();
    metadata.games = new const char*[metadata.games_count];
    for (auto i = 0; i < metadata.games_count; i++) {
        std::string& item = v_games[i];
        char* c_str = new char[item.length() + 1];
        std::strcpy(c_str, item.c_str());
        metadata.games[i] = c_str;
    }

    metadata.paths_count = v_paths.size();
    metadata.paths = new const char*[metadata.paths_count];
    for (auto i = 0; i < metadata.paths_count; i++) {
        std::string& item = v_paths[i];
        char* c_str = new char[item.length() + 1];
        std::strcpy(c_str, item.c_str());
        metadata.paths[i] = c_str;
    }
    return metadata;
}

static size_t chunk_pos;
static const char* chunk_data;

// Move an offset from current position in data.
void Move_Pos(int offset) {
    chunk_pos += offset;
}

enum ENDIAN {
    BIG = 0,
    LITTLE = 1
};

// Parses integer value from binary data.
template <std::integral INTEGRAL>
INTEGRAL Parse(ENDIAN endian) {
    INTEGRAL buffer;
    std::memcpy(&buffer, &chunk_data[chunk_pos], sizeof(buffer));
    (endian == BIG)
        ? buffer = toBigEndian(buffer)
        : buffer = toLittleEndian(buffer);
    chunk_pos += sizeof(buffer);
    return buffer;
}
// Parses string value from binary data.
// Use 0 to read until null character, or specify a length.
template <std::same_as<string> STRING>
auto Parse(int size) {
    STRING buffer = "";
    if (size > 0) {
        for (int i = 0; i < size; i++) {
            if (chunk_data[chunk_pos] != '\0') {
                buffer += chunk_data[chunk_pos];
            }
            chunk_pos++;
        }
    } else {
        for (int i = 0; chunk_data[chunk_pos] != '\0'; i++) {
            buffer += chunk_data[chunk_pos];
            chunk_pos++;
        }
        chunk_pos++;
    }
    return buffer;
}

// Dumps JSON data to a C string and returns via the heap.
char* JSON_to_C_Str(json& json_obj) {
    char* json_str = (char*)malloc(json_obj.dump().length() + 1);
    strcpy(json_str, json_obj.dump().c_str());
    return json_str;
}

// Frees all dynamically allocated memory.
void clean_up(const char** a_ptrs, int a_size, const char** b_ptrs, int b_size, const char* json_ptr) {
    for (int i = 0; i < a_size; i++) {
        delete(a_ptrs[i]);
    }
    delete(a_ptrs);
    for (int i = 0; i < b_size; i++) {
        delete(b_ptrs[i]);
    }
    delete(b_ptrs);
    delete(json_ptr);
}