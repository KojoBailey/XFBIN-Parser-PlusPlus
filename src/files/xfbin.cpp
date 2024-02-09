#include "../logger.h"
#include "files.h"

void BigEndian() {
    bigEndian = true;
}
void LittleEndian() {
    bigEndian = false;
}

std::string parseChar(int size, std::vector<char>& data, int& pos) {
    std::string buffer = "";
    if (size > 0) {
        for (int i = 0; i < size; i++) {
            if (data[pos] != '\0') {
                buffer += data[pos];
            }
            pos++;
        }
    } else {
        for (int i = 0; data[pos] != '\0'; i++) {
            buffer += data[pos];
            pos++;
        }
        pos++;
    }
    return buffer;
}

json Unpack_xfbin(std::string name, std::vector<char>& data) {
    Log log;
    log.file.open("Log.txt", std::ios::app);

    json JsonData;
    int pos = 0;

    std::string Magic = parseChar(4, data, pos);
    if (Magic != "NUCC") {
        log.Error("Magic \"NUCC\" not found. File is either encrypted or not a valid XFBIN.");
        End();
    }
    log.Send("Parsing...");
    
    BigEndian();
    DefInt(uint32_t, FileId);
    Skip(8);
    DefInt(uint32_t, ChunkTableSize);
    DefInt(uint32_t, MinPageSize);
    DefInt(uint16_t, FileId2);
    DefInt(uint16_t, XfbinId);

    // Chunk Table
    DefInt(uint32_t, ChunkTypeCount);
    DefInt(uint32_t, ChunkTypeSize);
    DefInt(uint32_t, FilePathCount);
    DefInt(uint32_t, FilePathSize);
    DefInt(uint32_t, ChunkNameCount);
    DefInt(uint32_t, ChunkNameSize);
    DefInt(uint32_t, ChunkMapCount);
    DefInt(uint32_t, ChunkMapSize);
    DefInt(uint32_t, ChunkMapIndicesCount);
    DefInt(uint32_t, ExtraIndicesCount);
        // Chunk Types
    std::vector<std::string> ChunkType;
    for (size_t i = 0; i < ChunkTypeCount; i++) {
        ChunkType.push_back(parseChar(0, data, pos));
    }
        // File Paths
    std::vector<std::string> FilePath;
    for (size_t i = 0; i < FilePathCount; i++) {
        FilePath.push_back(parseChar(0, data, pos));
    }
        // Chunk Names
    std::vector<std::string> ChunkName;
    for (size_t i = 0; i < ChunkNameCount; i++) {
        ChunkName.push_back(parseChar(0, data, pos));
    }
    pos--;
    Skip(4 - (pos % 4));
        // Chunk Maps
    struct ChunkMapData {
        uint32_t ChunkTypeIndex;
        uint32_t FilePathIndex;
        uint32_t ChunkNameIndex;
    }; 
    std::vector<ChunkMapData> ChunkMap;
    for (size_t i = 0; i < ChunkMapCount; i++) {
        ChunkMap.push_back({});
        parseInt(ChunkMap[i].ChunkTypeIndex, data, pos);
        parseInt(ChunkMap[i].FilePathIndex, data, pos);
        parseInt(ChunkMap[i].ChunkNameIndex, data, pos);
    }
        // Chunk Map Indices
    std::vector<uint32_t> ChunkMapIndex;
    for (size_t i = 0; i < ChunkMapIndicesCount; i++) {
        ChunkMapIndex.push_back(0);
        parseInt(ChunkMapIndex[i], data, pos);
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

    log.file.close();
    return JsonData;
}

void ExtractData(std::vector<char>& data, json XfbinJson, std::ofstream& out_file) {
    int pos = 1276;
    BigEndian();
    DefInt(uint32_t, ChunkSize);
    Skip(8);

    for (size_t i = 0; i < ChunkSize; i++) {
        out_file << data[pos];
        pos++;
    }
}