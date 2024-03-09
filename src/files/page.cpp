#include "../logger.h"
#include "files.h"

void Page::Unpack(size_t index) {
    LOG_VERBOSE("Attempting to unpack page " + to_string(index) + "...");

    // Unpack chunk data.
    for (size_t i = 0; true; i++) {
        LOG_VERBOSE("Unpacking chunk " + to_string(i) + " from page " + to_string(index) + ".");
        chunks.push_back({});
        chunks[i].Load_Binary(chunk_map_offset, extra_map_offset);

        // Store metadata to page JSON.
        auto& chunk_metadata = metadata["Chunk " + to_string(i)];
        chunk_metadata["Type"] = chunks[i].type;
        chunk_metadata["Name"] = chunks[i].name;
        chunk_metadata["Path"] = chunks[i].path;

        // If nuccChunkPage reached, end the loop.
        if (chunks[i].type == "nuccChunkPage") {
            LOG_DEBUG("Found `nuccChunkPage`. Breaking loop...");
            break;
        }
    }

    // Create directory and its files.
        // 
    int page_title_index;
    for (int i = 0; true; i++) {
        if (chunks[i].type != "nuccChunkPage" && chunks[i].type != "nuccChunkNull") {
            page_title_index = i;
            break;
        }
    }
        // Create directory.
    fs::path directory = xfbin.name + "\\[" + Format_3_Digits(index) + "] " + chunks[page_title_index].name + " (" + chunks[page_title_index].type + ")";
    LOG_VERBOSE("Creating directory `" + directory.string() + "`...");
    fs::create_directory(directory);
    if (!fs::exists(directory)) {
        LOG_CRITICAL("Failed to create page directory.");
    }
        // Create chunk file.
    for (size_t i = 0; i < chunks.size(); i++) {
        if (chunks[i].type != "nuccChunkPage" && chunks[i].type != "nuccChunkNull") {
            LOG_VERBOSE("Creating chunk file...");
            chunks[i].Unpack(directory);
        }
    }
        // Create and output metadata to `_page.json`.
    metadata["Chunk Map Offset"] = chunk_map_offset;
    metadata["Extra Map Offset"] = extra_map_offset;
    output_file.open(directory / "_page.json");
    output_file << metadata.dump(2);
    output_file.close();
        // Increase global map index offsets.
    xfbin.chunk_map_offset += chunk_map_offset;
    LOG_VERBOSE("Increased chunk map offset by " + to_string(chunk_map_offset) + ".");
    xfbin.extra_map_offset += extra_map_offset;
    LOG_VERBOSE("Increased extra map offset by " + to_string(extra_map_offset) + ".");
}