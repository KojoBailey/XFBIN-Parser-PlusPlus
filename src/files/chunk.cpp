#include "../logger.h"
#include "files.h"

#include <Windows.h>

void Chunk::Load_Binary(uint32_t& chunk_map_offset, uint32_t& extra_map_index) {
    // Define metadata.
    LOG_DEBUG("Position: " + to_string(file_pos));
    BigEndian();
    Parse_Int(size, xfbin.vector_data);
    Define_Int(uint32_t, chunk_map_index);
    Define_Int(uint16_t, version);
    Define_Int(uint16_t, field_0A);

    // Use XFBIN chunk maps to determine chunk information.
    auto& chunk_table = xfbin.metadata["Chunk Table"];
    int chunk_map_pointer = chunk_table["Chunk Map Indices"]["Chunk Map Index " + to_string(chunk_map_index + xfbin.chunk_map_offset)];
    auto& chunk_map = chunk_table["Chunk Maps"]["Chunk Map " + to_string(chunk_map_pointer)];
    auto buffer = chunk_table["Chunk Types"]["Chunk Type " + to_string(chunk_map["Chunk Type Index"].j_get(int))];
    type = (buffer == nullptr) ? "" : buffer;
    buffer = chunk_table["File Paths"]["File Path " + to_string(chunk_map["File Path Index"].j_get(int))];
    path = (buffer == nullptr) ? "" : buffer;
    buffer = chunk_table["Chunk Names"]["Chunk Name " + to_string(chunk_map["Chunk Name Index"].j_get(int))];
    name = (buffer == nullptr) ? "" : buffer;

    if (type == "nuccChunkPage") {
        // Read page information.
        Parse_Int(chunk_map_offset, xfbin.vector_data);
        Parse_Int(extra_map_index, xfbin.vector_data);
    } else {
        // Read binary data to struct.
        for (int i = 0; i < size; i++) {
            vector_data.push_back(xfbin.vector_data[file_pos++]);
        }
    }
}

void Chunk::Unpack(fs::path directory) {
    if (type == "nuccChunkBinary") {
        plugin_metadata this_metadata;
        for ( const auto& file : fs::recursive_directory_iterator("plugins") ) {
            // Load DLL library.
            LOG_VERBOSE("Attempting to load library `" + file.path().filename().string() + "`...");
            HMODULE library = LoadLibrary(file.path().string().c_str());
            if (library == nullptr) {
                LOG_CRITICAL("Failed to open library `" + file.path().filename().string() + "`.");
            }

            // Get DLL metadata.
            LOG_VERBOSE("Reading DLL metadata...");
            const auto get_plugin_metadata = (plugin_metadata (*)())GetProcAddress(library, "get_plugin_metadata");
            if (get_plugin_metadata == nullptr) {
                LOG_ERROR("Failed to fetch function `get_plugin_metadata()` from library `" + file.path().filename().string() + "`. Skipping...");
                break;
            }
            this_metadata = get_plugin_metadata();

            // Unpack and return if metadata matches chunk. Otherwise, move on to next DLL.
            if ( Does_Array_Contain(this_metadata.games, this_metadata.games_count, game) || game == "" ) {
                if ( Does_Array_Contain(this_metadata.paths, this_metadata.paths_count, path) ) {
                    LOG_VERBOSE("Unpacking chunk data...");
                    const auto unpack = ( const char* (*)(const char* array_data) )GetProcAddress(library, "unpack");
                    if (unpack == nullptr) {
                        LOG_ERROR("Failed to fetch function `unpack()` from library `" + file.path().filename().string() + "`. Skipping...");
                        break;
                    }
                    const char* json_ptr = unpack(vector_data.data());
                    json_data = json::parse(std::string(json_ptr));
                    
                    // Free dynamically allocated memory.
                    const auto clean_up = (void (*)(const char**, size_t, const char**, size_t, const char*))GetProcAddress(library, "clean_up");
                    if (clean_up == nullptr) {
                        LOG_ERROR("Failed to fetch function `clean_up()` from library `" + file.path().filename().string() + "`. Potential memory leaks.");
                    } else {
                        LOG_VERBOSE("Freeing DLL dynamically-allocated memory...");
                        clean_up(this_metadata.games, this_metadata.games_count, this_metadata.paths, this_metadata.paths_count, json_ptr);
                    }
                    break;
                }
            }
            LOG_VERBOSE("Freeing library...");
            FreeLibrary(library);
        }

        if (json_data.is_null()) {
            LOG_WARN("No plugin found to parse " + name + " binary.");
            output_file.open(directory / (name + ".binary"), std::ios::binary);
            Write_Vector_To_File(output_file, vector_data);
        } else {
            output_file.open(directory / (name + ".json"));
            output_file << json_data.dump(2);
            LOG_VERBOSE("Chunk data converted to JSON.");
        }

    } else {
        output_file.open(directory / (name + ".binary"), std::ios::binary);
        Write_Vector_To_File(output_file, vector_data);
        LOG_VERBOSE("Chunk data extracted as binary.");
    }
}

/*
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