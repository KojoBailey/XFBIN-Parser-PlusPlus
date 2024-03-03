#include "../logger.h"
#include "files.h"

#include <windows.h>

void Chunk::Load_Binary(uint32_t& chunk_map_offset, uint32_t& extra_map_index) {
    // Define metadata.
    LOG_DEBUG("Position: " + str(file_pos));
    BigEndian();
    Parse_Int(size, xfbin.vector_data);
    Define_Int(uint32_t, chunk_map_index);
    Define_Int(uint16_t, version);
    Define_Int(uint16_t, field_0A);

    // Use XFBIN chunk maps to determine chunk information.
    auto& chunk_table = xfbin.metadata["Chunk Table"];
    int chunk_map_pointer = chunk_table["Chunk Map Indices"]["Chunk Map Index " + str(chunk_map_index + xfbin.chunk_map_offset)];
    auto& chunk_map = chunk_table["Chunk Maps"]["Chunk Map " + str(chunk_map_pointer)];
    auto buffer = chunk_table["Chunk Types"]["Chunk Type " + str(chunk_map["Chunk Type Index"].j_get(int))];
    type = (buffer == nullptr) ? "" : buffer;
    buffer = chunk_table["File Paths"]["File Path " + str(chunk_map["File Path Index"].j_get(int))];
    path = (buffer == nullptr) ? "" : buffer;
    buffer = chunk_table["Chunk Names"]["Chunk Name " + str(chunk_map["Chunk Name Index"].j_get(int))];
    name = (buffer == nullptr) ? "" : buffer;

    if (type == "nuccChunkPage") {
        // Read page information.
        Parse_Int(chunk_map_offset, xfbin.vector_data);
        Parse_Int(extra_map_index, xfbin.vector_data);
    } else {
        // Read binary data to struct.
        for (int i = 0; i < size; i++) {
            vector_data.push_back(xfbin.vector_data[file_pos]);
            array_data = vector_data.data();
            file_pos++;
        }
    }
}

void Chunk::Unpack(fs::path directory) {
    output_file.open(directory / (name + ".binary"), std::ios::binary);
    for (int i = 0; i < vector_data.size(); i++) {
        output_file << vector_data[i];
    }
}

/*
bool does_vector_contain(const char** arr, size_t count, std::string value) {
    for (size_t i = 0; i < count; i++) {
        if (arr[i] == value) {
            return true;
        }
    }
    return false;
}

const char* vector_to_cstring(std::vector<char>& vec) {
    char* buffer = new char[vec.size() + 1];

    for (size_t i = 0; i < vec.size(); i++) {
        buffer[i] = vec[i];
    }

    buffer[vec.size()] = '\0';

    return buffer;
}

json UnpackChunkData(std::vector<char>& chunk_data, json& XfbinJson, size_t index, std::string chunk_path, std::string game) {
    plugin_metadata this_metadata;
    for ( const auto& file : fs::recursive_directory_iterator("plugins") ) {
        // Load DLL library.
        auto library = LoadLibrary(file.path().string().c_str());

        // Get DLL metadata.
        const auto get_plugin_metadata = ( plugin_metadata (*)() )GetProcAddress(library, "get_plugin_metadata");
        this_metadata = get_plugin_metadata();

        // Unpack and return if metadata matches chunk. Otherwise, move on to next DLL.
        if ( does_vector_contain(this_metadata.games, this_metadata.games_count, game) || game == "" ) {
            if ( does_vector_contain(this_metadata.paths, this_metadata.paths_count, chunk_path) ) {
                const auto unpack = ( json (*)(const char* chunk_data, const char* XfbinJson, int index) )GetProcAddress(library, "unpack");
                return unpack( vector_to_cstring(chunk_data), XfbinJson.dump().c_str(), index );
            }
        }
        FreeLibrary(library);
    }
    return nullptr;
}
*/