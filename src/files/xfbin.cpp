#include "../logger.h"
#include "files.h"

int Pos = 0;

void BigEndian() {
    bigEndian = true;
}
void LittleEndian() {
    bigEndian = false;
}

std::string parseVectorChar(int size, std::vector<char>& data) {
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
std::string Format3Digits(int number) {
    if (number < 10)
        return "00" + std::to_string(number);
    else if (number < 100)
        return "0" + std::to_string(number);
    else
        return std::to_string(number);
}

json Unpack_xfbin(std::string name, std::vector<char>& data) {
    Log log;
    log.file.open("Log.txt", std::ios::app);

    json JsonData;

    std::string Magic = parseVectorChar(4, data);
    if (Magic != "NUCC") {
        log.Error("Magic \"NUCC\" not found. File is either encrypted or not a valid XFBIN.");
        End();
    }
    log.Send("Parsing XFBIN metadata...");

    struct ExtraMapIndices {
        uint32_t ExtraNameIndex;
        uint32_t ExtraMapIndex;
    };
    
    BigEndian();
    DefineVectorInt(uint32_t, FileId);
    Skip(8);
    DefineVectorInt(uint32_t, ChunkTableSize);
    DefineVectorInt(uint32_t, MinPageSize);
    DefineVectorInt(uint16_t, FileId2);
    DefineVectorInt(uint16_t, XfbinId);

    // Chunk Table
    DefineVectorInt(uint32_t, ChunkTypeCount);
    DefineVectorInt(uint32_t, ChunkTypeSize);
    DefineVectorInt(uint32_t, FilePathCount);
    DefineVectorInt(uint32_t, FilePathSize);
    DefineVectorInt(uint32_t, ChunkNameCount);
    DefineVectorInt(uint32_t, ChunkNameSize);
    DefineVectorInt(uint32_t, ChunkMapCount);
    DefineVectorInt(uint32_t, ChunkMapSize);
    DefineVectorInt(uint32_t, ChunkMapIndicesCount);
    DefineVectorInt(uint32_t, ExtraIndicesCount);
        // Chunk Types
    std::vector<std::string> ChunkType;
    for (size_t i = 0; i < ChunkTypeCount; i++) {
        ChunkType.push_back(parseVectorChar(0, data));
    }
        // File Paths
    std::vector<std::string> FilePath;
    for (size_t i = 0; i < FilePathCount; i++) {
        FilePath.push_back(parseVectorChar(0, data));
    }
        // Chunk Names
    std::vector<std::string> ChunkName;
    for (size_t i = 0; i < ChunkNameCount; i++) {
        ChunkName.push_back(parseVectorChar(0, data));
    }
    Pos--;
    Skip(4 - (Pos % 4));
        // Chunk Maps
    struct ChunkMapData {
        uint32_t ChunkTypeIndex;
        uint32_t FilePathIndex;
        uint32_t ChunkNameIndex;
    }; 
    std::vector<ChunkMapData> ChunkMap;
    for (size_t i = 0; i < ChunkMapCount; i++) {
        ChunkMap.push_back({});
        parseVectorInt(ChunkMap[i].ChunkTypeIndex, data);
        parseVectorInt(ChunkMap[i].FilePathIndex, data);
        parseVectorInt(ChunkMap[i].ChunkNameIndex, data);
    }
        // Extra Map Indices
    std::vector<ExtraMapIndices> ExtraMapIndex;
    for (size_t i = 0; i < ExtraIndicesCount; i++) {
        ExtraMapIndex.push_back({});
        parseVectorInt(ExtraMapIndex[i].ExtraNameIndex, data);
        parseVectorInt(ExtraMapIndex[i].ExtraMapIndex, data);
    }
        // Chunk Map Indices
    std::vector<uint32_t> ChunkMapIndex;
    for (size_t i = 0; i < ChunkMapIndicesCount; i++) {
        ChunkMapIndex.push_back(0);
        parseVectorInt(ChunkMapIndex[i], data);
    }
    
    // Pages
    struct Pages {
        struct Chunks {
            uint32_t Size;
            uint32_t ChunkMapIndex;
            uint16_t Version;
            uint16_t Field0A;
        };
        std::vector<Chunks> Chunk;
    };
    std::vector<Pages> Page;
    
    
    JsonData = {
        {"Filename", name},
        {"Magic", Magic},
        {"File ID", FileId},
        {"Chunk Table Size", ChunkTableSize},
        {"Minimum Page Size", MinPageSize},
        {"File ID 2", FileId2},
        {"XFBIN ID", XfbinId},
        {"Chunk Table", {
            {"Chunk Type Count", ChunkTypeCount},
            {"Chunk Type Size", ChunkTypeSize},
            {"File Path Count", FilePathCount},
            {"File Path Size", FilePathSize},
            {"Chunk Name Count", ChunkNameCount},
            {"Chunk Name Size", ChunkNameSize},
            {"Chunk Map Count", ChunkMapCount},
            {"Chunk Map Size", ChunkMapSize},
            {"Chunk Map Indices Count", ChunkMapIndicesCount},
            {"Extra Indices Count", ExtraIndicesCount}
        }}
    };
    auto& JsonChunkTable = JsonData["Chunk Table"];
    for (size_t i = 0; i < ChunkTypeCount; i++) {
        JsonChunkTable["Chunk Types"]["Chunk Type " + str(i)] = ChunkType[i];
    }
    for (size_t i = 0; i < FilePathCount; i++) {
        JsonChunkTable["File Paths"]["File Path " + str(i)] = FilePath[i];
    }
    for (size_t i = 0; i < ChunkNameCount; i++) {
        JsonChunkTable["Chunk Names"]["Chunk Name " + str(i)] = ChunkName[i];
    }
    for (size_t i = 0; i < ChunkMapCount; i++) {
        JsonChunkTable["Chunk Maps"]["Chunk Map " + str(i)] = {
            {"Chunk Type Index", ChunkMap[i].ChunkTypeIndex},
            {"File Path Index", ChunkMap[i].FilePathIndex},
            {"Chunk Name Index", ChunkMap[i].ChunkNameIndex}
        };
    }
    for (size_t i = 0; i < ChunkMapIndicesCount; i++) {
        JsonChunkTable["Chunk Map Indices"]["Chunk Map Index " + str(i)] = ChunkMapIndex[i];
    }
    for (size_t i = 0; i < ExtraIndicesCount; i++) {
        JsonChunkTable["Extra Map Indices"]["Extra Map Indices " + str(i)]["Extra Name Index"] = ExtraMapIndex[i].ExtraNameIndex;
        JsonChunkTable["Extra Map Indices"]["Extra Map Indices " + str(i)]["Extra Map Index"] = ExtraMapIndex[i].ExtraMapIndex;
    }

    log.Success("XFBIN metadata output as \"_xfbin.json\".");
    log.file.close();
    return JsonData;
}

chunk_struct Unpack_chunkInfo(std::vector<char>& data, json& XfbinJson, size_t& index_offset, int page_i) {
    chunk_struct StructData;

    // Define variables
    BigEndian();
    DefineVectorInt(uint32_t, Size);
    DefineVectorInt(uint32_t, ChunkMapIndex);
    DefineVectorInt(uint16_t, Version);
    DefineVectorInt(uint16_t, Field0A);

    auto& ChunkTable = XfbinJson["Chunk Table"];
    int ChunkMapPointer = ChunkTable["Chunk Map Indices"]["Chunk Map Index " + str(ChunkMapIndex + index_offset)];
    auto& ChunkMap = ChunkTable["Chunk Maps"]["Chunk Map " + str(ChunkMapPointer)];

    // Define data into struct
    auto tempType = ChunkTable["Chunk Types"]["Chunk Type " + str(ChunkMap["Chunk Type Index"].j_get(int))];
    StructData.Type = (tempType == nullptr) ? "" : tempType;
    auto tempPath = ChunkTable["File Paths"]["File Path " + str(ChunkMap["File Path Index"].j_get(int))];
    StructData.Path = (tempPath == nullptr) ? "" : tempPath;
    auto tempName = ChunkTable["Chunk Names"]["Chunk Name " + str(ChunkMap["Chunk Name Index"].j_get(int))];
    StructData.Name = (tempName == nullptr) ? "" : tempName;

    std::string Title = "[" + Format3Digits(page_i) + "] " + ((StructData.Type == "nuccChunkNull") ? "nuccChunkNull" : (StructData.Name + " (" + StructData.Type + ")"));

    // Define JSON table
    StructData.JsonData = {
        {Title, {
            {"Size", Size},
            {"Chunk Map Index", ChunkMapIndex},
            {"Version", Version},
            {"Field 0A", Field0A},
            {"Chunk Map Information", {
                {"Chunk Type", StructData.Type},
                {"File Path", StructData.Path},
                {"Chunk Name", StructData.Name}
            }}
        }}
    };

    return StructData;
}

json ExtractPageData(std::vector<char>& data, json& XfbinJson, size_t& map_index_offset, size_t& extra_index_offset) {
    json PageChunkJson, ExtraIndicesJson;

    BigEndian();
    DefineVectorInt(uint32_t, IndexOffset);
    DefineVectorInt(uint32_t, ExtraIndexOffset);

    PageChunkJson = {
        {"Chunk Map Index Offset", IndexOffset},
        {"Extra Map Index Offset", ExtraIndexOffset}
    };

    if (ExtraIndexOffset > 0) {
        auto& XfbinExtra = XfbinJson["Chunk Table"]["Extra Map Indices"];
        for (int i = 0; i < ExtraIndexOffset; i++) {
            ExtraIndicesJson["Extra Map Indices"]["Extra Indices " + str(i)]["Extra Name"] = XfbinJson["Chunk Table"]["Chunk Names"]["Chunk Name " +
                str(XfbinExtra["Extra Map Indices " + str(i + extra_index_offset)]["Extra Name Index"].j_get(int))
            ];
            ExtraIndicesJson["Extra Map Indices"]["Extra Indices " + str(i)]["Extra Map"] = XfbinExtra["Extra Map Indices " + str(i + extra_index_offset)]["Extra Map Index"];
        }
        PageChunkJson.merge_patch(ExtraIndicesJson);
    }

    map_index_offset += IndexOffset;
    extra_index_offset += ExtraIndexOffset;
    return PageChunkJson;
}

void ExtractData(std::vector<char>& data, json XfbinJson, std::ofstream& out_file) {
    Skip(-12);
    BigEndian();
    DefineVectorInt(uint32_t, ChunkSize);
    Skip(8);

    for (size_t i = 0; i < ChunkSize; i++) {
        out_file << data[Pos];
        Pos++;
    }
}