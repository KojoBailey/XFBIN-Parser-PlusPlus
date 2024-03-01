// sndcmnparam.dll

#include "dll.h"

int file_pos;

plugin_metadata get_plugin_metadata() {
    std::vector<const char*> v_games = {"ASB", "ASBR"};
    std::vector<const char*> v_paths = {"sndcmnparam", "battle"};

    return create_metadata(v_games, v_paths);
}

const char* unpack(const char* data, const char* str_xfbin_json, int index) {
    json XfbinJson = json::parse(str_xfbin_json);
    json OutputJson;

    BigEndian();
    DefineInt(uint32_t, Size);
    LittleEndian();
    DefineInt(uint16_t, Count);

    auto& ChunkTable = XfbinJson["Chunk Table"];
    std::string Type = str(ChunkTable["Chunk Maps"]["Chunk Map " + str(index)]["Chunk Type Index"].template get<int>());
    OutputJson = {
        {"Metadata", {
            {"Name", ChunkTable["Chunk Names"]["Chunk Name " + str(index)]},
            {"Type", ChunkTable["Chunk Types"]["Chunk Type " + Type]},
            {"Path", ChunkTable["File Paths"]["File Path " + str(index)]}
        }}
    };

    for (int i = 0; i < Count; i++) {
        OutputJson["Entry " + str(i + 1)] = parse_array_char(32, data);
    }

    return OutputJson.dump().c_str();
}