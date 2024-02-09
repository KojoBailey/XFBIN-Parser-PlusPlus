#include "files.h"

json Unpack_ASBR_sndcmnparam(std::vector<char>& data, json XfbinJson, int index) {
    json JsonData;

    int pos = 1288; // get xfbin parsing
    BigEndian();
    uint32_t Size; parseInt(Size, data, pos);
    LittleEndian();
    uint16_t Count; parseInt(Count, data, pos);

    std::string Type = str(XfbinJson["Chunk Table"]["Chunk Maps"]["Chunk Map " + str(index)]["Chunk Type Index"].template get<int>());
    JsonData = {
        {"Metadata", {
            {"Name", XfbinJson["Chunk Table"]["Chunk Names"]["Chunk Name " + str(index)]},
            {"Type", XfbinJson["Chunk Table"]["Chunk Types"]["Chunk Type " + Type]},
            {"Path", XfbinJson["Chunk Table"]["File Paths"]["File Path " + str(index)]}
        }}
    };

    std::vector<std::string> ID;
    for (int i = 0; i < Count; i++) {
        ID.push_back({parseChar(32, data, pos)});
        JsonData["Entry " + str(i + 1)] = ID[i];
    }

    return JsonData;
}