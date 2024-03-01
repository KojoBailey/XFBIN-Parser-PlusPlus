#include "files.h"

#include <windows.h>

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