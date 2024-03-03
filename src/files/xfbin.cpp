#include "../logger.h"
#include "files.h"

void XFBIN::Unpack() {
    // Create main directory.
    LOG_INFO("Creating main directory `" + name + "`...");
    fs::path main_directory = name;
    fs::create_directory(main_directory);

    // Create JSON for metadata and parse into it.
    output_file.open(main_directory / "_xfbin.json");
    if (!output_file.is_open()) {
        LOG_CRITICAL("Failed to create `_xfbin.json`.");
    }
    Read_Metadata();
    output_file << metadata.dump(2);
    output_file.close();

    // Create pages until end of file reached.
    LOG_INFO("Unpacking page data...");
    for (size_t i = 0; true; i++) {
        // Unpack page data.
        pages.push_back({});
        pages[i].Unpack(i);
        
        // If end of file reached, exit loop.
        if ( !(file_pos < vector_data.size()) ) {
            LOG_VERBOSE("End of file detected at " + str(file_pos) + " bytes.");
            break;
        }
    }

    LOG_SUCCESS("Parsing complete!");
}

void XFBIN::Read_Metadata() {
    // Check XFBIN file magic in case of encryption, corruption, etc.
    LOG_INFO("Parsing XFBIN metadata to `_xfbin.json`...");
    std::string magic = Parse_String(4, vector_data);
    if (magic != "NUCC") {
        LOG_CRITICAL("Magic `NUCC` not found. File is either encrypted or not a valid XFBIN.");
    }

    BigEndian();
    LOG_VERBOSE("Parsing XFBIN header information...");
    Define_Int(uint32_t, file_id);
    FSkip(8);
    Define_Int(uint32_t, chunk_table_size);
    Define_Int(uint32_t, minimum_page_size);
    Define_Int(uint16_t, file_id_2);
    Define_Int(uint16_t, xfbin_id);

    // Chunk Table
    LOG_VERBOSE("Parsing XFBIN chunk table...");
    Define_Int(uint32_t, chunk_type_count);
    Define_Int(uint32_t, chunk_type_size);
    Define_Int(uint32_t, file_path_count);
    Define_Int(uint32_t, file_path_size);
    Define_Int(uint32_t, chunk_name_count);
    Define_Int(uint32_t, chunk_name_size);
    Define_Int(uint32_t, chunk_map_count);
    Define_Int(uint32_t, chunk_map_size);
    Define_Int(uint32_t, chunk_map_index_count);
    Define_Int(uint32_t, extra_map_indices_count);
        // Chunk Types
    LOG_VERBOSE("Parsing chunk type information...");
    std::vector<std::string> chunk_type;
    for (size_t i = 0; i < chunk_type_count; i++) {
        chunk_type.push_back(Parse_String(0, vector_data));
    }
        // File Paths
    LOG_VERBOSE("Parsing file path information...");
    std::vector<std::string> file_path;
    for (size_t i = 0; i < file_path_count; i++) {
        file_path.push_back(Parse_String(0, vector_data));
    }
        // Chunk Names
    LOG_VERBOSE("Parsing chunk name information...");
    std::vector<std::string> chunk_name;
    for (size_t i = 0; i < chunk_name_count; i++) {
        chunk_name.push_back(Parse_String(0, vector_data));
    }
    file_pos--;
    FSkip(4 - (file_pos % 4));
        // Chunk Maps
    LOG_VERBOSE("Parsing chunk map information...");
    struct Chunk_Map_Data {
        uint32_t chunk_type_index;
        uint32_t file_path_index;
        uint32_t chunk_name_index;
    }; 
    std::vector<Chunk_Map_Data> chunk_map;
    for (size_t i = 0; i < chunk_map_count; i++) {
        chunk_map.push_back({});
        Parse_Int(chunk_map[i].chunk_type_index, vector_data);
        Parse_Int(chunk_map[i].file_path_index, vector_data);
        Parse_Int(chunk_map[i].chunk_name_index, vector_data);
    }
        // Extra Map Indices
    LOG_VERBOSE("Parsing extra map index information...");
    struct Extra_Map_Indices {
        uint32_t extra_name_index;
        uint32_t extra_map_index;
    };
    std::vector<Extra_Map_Indices> extra_map_indices;
    for (size_t i = 0; i < extra_map_indices_count; i++) {
        extra_map_indices.push_back({});
        Parse_Int(extra_map_indices[i].extra_name_index, vector_data);
        Parse_Int(extra_map_indices[i].extra_map_index, vector_data);
    }
        // Chunk Map Indices
    LOG_VERBOSE("Parsing chunk map index information...");
    std::vector<uint32_t> chunk_map_index;
    for (size_t i = 0; i < chunk_map_index_count; i++) {
        chunk_map_index.push_back(0);
        Parse_Int(chunk_map_index[i], vector_data);
    }

    LOG_VERBOSE("Writing parsed data to JSON...");
    metadata = {
        {"Filename", name},
        {"Magic", magic},
        {"File ID", file_id},
        {"Chunk Table Size", chunk_table_size},
        {"Minimum Page Size", minimum_page_size},
        {"File ID 2", file_id_2},
        {"XFBIN ID", xfbin_id},
        {"Chunk Table", {
            {"Chunk Type Count", chunk_type_count},
            {"Chunk Type Size", chunk_type_size},
            {"File Path Count", file_path_count},
            {"File Path Size", file_path_size},
            {"Chunk Name Count", chunk_name_count},
            {"Chunk Name Size", chunk_name_size},
            {"Chunk Map Count", chunk_map_count},
            {"Chunk Map Size", chunk_map_size},
            {"Chunk Map Indices Count", chunk_map_index_count},
            {"Extra Map Indices Count", extra_map_indices_count}
        }}
    };
    auto& json_chunk_table = metadata["Chunk Table"];
    for (size_t i = 0; i < chunk_type_count; i++) {
        json_chunk_table["Chunk Types"]["Chunk Type " + str(i)] = chunk_type[i];
    }
    for (size_t i = 0; i < file_path_count; i++) {
        json_chunk_table["File Paths"]["File Path " + str(i)] = file_path[i];
    }
    for (size_t i = 0; i < chunk_name_count; i++) {
        json_chunk_table["Chunk Names"]["Chunk Name " + str(i)] = chunk_name[i];
    }
    for (size_t i = 0; i < chunk_map_count; i++) {
        json_chunk_table["Chunk Maps"]["Chunk Map " + str(i)] = {
            {"Chunk Type Index", chunk_map[i].chunk_type_index},
            {"File Path Index", chunk_map[i].file_path_index},
            {"Chunk Name Index", chunk_map[i].chunk_name_index}
        };
    }
    for (size_t i = 0; i < chunk_map_index_count; i++) {
        json_chunk_table["Chunk Map Indices"]["Chunk Map Index " + str(i)] = chunk_map_index[i];
    }
    for (size_t i = 0; i < extra_map_indices_count; i++) {
        json_chunk_table["Extra Map Indices"]["Extra Map Indices " + str(i)]["Extra Name Index"] = extra_map_indices[i].extra_name_index;
        json_chunk_table["Extra Map Indices"]["Extra Map Indices " + str(i)]["Extra Map Index"] = extra_map_indices[i].extra_map_index;
    }

    chunk_map_offset = 0;
    extra_map_offset = 0;

    LOG_SUCCESS("Finished parsing XFBIN metadata.");
}

void XFBIN::Repack() {

}

/*
chunk_struct Unpack_chunkInfo(std::vector<char>& vector_data, json& XfbinJson, size_t& index_offset, int page_i) {
    chunk_struct StructData;

    // Define variables
    BigEndian();
    Define_Int(uint32_t, Size);
    Define_Int(uint32_t, ChunkMapIndex);
    Define_Int(uint16_t, Version);
    Define_Int(uint16_t, Field0A);

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

json ExtractPageData(std::vector<char>& vector_data, json& XfbinJson, size_t& map_index_offset, size_t& extra_index_offset) {
    json PageChunkJson, ExtraIndicesJson;

    BigEndian();
    Define_Int(uint32_t, IndexOffset);
    Define_Int(uint32_t, ExtraIndexOffset);

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

void ExtractData(std::vector<char>& vector_data, json XfbinJson, std::ofstream& out_file) {
    FSkip(-12);
    BigEndian();
    Define_Int(uint32_t, ChunkSize);
    FSkip(8);

    for (size_t i = 0; i < ChunkSize; i++) {
        out_file << vector_data[file_pos];
        file_pos++;
    }
}
*/