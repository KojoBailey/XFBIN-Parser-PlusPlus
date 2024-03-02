#pragma once

#include "../logger.h"

#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>
#include <nlohmann/json.hpp>
#include <cstdint>
#include <unordered_map>
#include <bit>

// Short-handing.
#define str(x) std::to_string(x)
namespace fs = std::filesystem;
using json = nlohmann::ordered_json;
#define j_get(Type) template get<Type>()

// Keep track of file data position.
extern int file_pos;

// Keep track of global parser endianness.
inline bool bigEndian;
#define IS_BIG_ENDIAN (std::endian::native == std::endian::big)

// Set global parser endianness.
void BigEndian();
void LittleEndian();

// Convert specific values to different endians.
template <std::integral T>
T toBigEndian(T value) {
    if (!IS_BIG_ENDIAN) return std::byteswap(value); else return value;
}
template <std::integral T>
T toLittleEndian(T value) {
    if (IS_BIG_ENDIAN) return std::byteswap(value); else return value;
}

// Macros for writing parser templates.
#define DefineVectorInt(type, var) \
    type var; \
    parseVectorInt(var, vector_data)
#define FSkip(val) file_pos+=val

// Functions for parsing vector data.
template <std::integral T>
void parseVectorInt(T& var, std::vector<char>& vector_data) {
    std::memcpy(&var, &vector_data[file_pos], sizeof(var));
    if (bigEndian == true) var = toBigEndian(var); else var = toLittleEndian(var);
    file_pos += sizeof(var);
}
std::string parseVectorChar(int size, std::vector<char>& vector_data);

// Format a number as 3 digits with leading 0s.
std::string Format3Digits(int number);

// Functions for unpacking and extracting data.
json Get_XFBIN_Meta(std::string name, std::vector<char>& vector_data);
struct chunk_struct {
	json JsonData;
	std::string Size;
	std::string Type;
	std::string Path;
	std::string Name;
};
chunk_struct Unpack_chunkInfo(std::vector<char>& vector_data, json& XfbinJson, size_t& map_index_offset, int chunk_i);
json ExtractPageData(std::vector<char>& vector_data, json& XfbinJson, size_t& map_index_offset, size_t& extra_index_offset);
void ExtractData(std::vector<char>& vector_data, json XfbinJson, std::ofstream& out_file);
json UnpackChunkData(std::vector<char>& vector_data, json& XfbinJson, size_t index, std::string chunk_path, std::string game);
struct plugin_metadata {
    const char** games;
	size_t games_count;
    const char** paths;
	size_t paths_count;
};

void Read_File_To_Vector(std::ifstream& file, std::vector<char>& vector_data) {
	vector_data.clear();
	while (file.peek() != EOF) {
		vector_data.push_back(file.get());
	}
}

// Class for individual XFBIN page chunks.
class Chunk {
public:
    std::string name;               // Internal name (not literal XFBIN name).
    std::string path;               // Internal game file path (not literal XFBIN path).
    std::string type;               // e.g. "nuccChunkBinary".
    std::vector<char> vector_data;  // Individual byte data stored to vector.
    const char* array_data;         // Individual byte data stored to array for C compatability.
    size_t size;                    // Size of chunk in bytes, used to determine the array size too.
    json json_data;                 // Only for binary data, represented by JSON.
};

// Class for individual XFBIN pages.
class Page {
public:
    uint32_t chunk_map_offset;      // Offset size from chunk map indices, used for later pages.
    uint32_t extra_map_offset;      // Offset size from extra map indices, used for later pages.
    json metadata;                  // Metadata stored to JSON.
    std::vector<Chunk> chunks;		// XFBIN chunks. Uses objects from the Chunk class.
};

// Class for XFBIN file information, whether the input is an XFBIN or an unpacked directory.
class XFBIN {
public:
    std::vector<Page> pages;		// XFBIN pages. Uses objects from the Page class.
    std::ofstream output_file;      // Will be the opposite of input filetype.

	void Load_XFBIN(fs::path path, std::string type) {
		// Open file into input_file.
		input_file.open(path);
		input_type = type;

		// Check if file succeeded in opening.
		if (!input_file.is_open()) {
			if (type == "xfbin") {
				LOG_CRITICAL("XFBIN failed to open from path `" + path.string() + "`.");
			} else {
				LOG_CRITICAL("`_xfbin.json` not found in unpacked directory `" + path.string() + "`.");
			}
		}

		// Set input_type depending on input file extension.
		if (input_type == "xfbin") {
			name = path.stem().string();
		} else if (input_type == "json") {
			input_type = "json";
			metadata = json::parse(input_file);
			if (metadata["Filename"].is_null()) {
				LOG_CRITICAL("Filename could not be found in _xfbin.json");
			} else {
				name = metadata["Filename"];
			}
		}

		// Read file bytes to vector_data for faster access later.
		LOG_VERBOSE("Reading file data to vector...");
		Read_File_To_Vector(input_file, vector_data);
		// Close input file as no longer needed.
		input_file.close();

		LOG_SUCCESS("XFBIN data loaded.");
	}

private:
	std::ifstream input_file;       // Can be XFBIN (for unpacking) or JSON (for repacking).
    std::string input_type;         // XFBIN or JSON depending on input.
    std::string name;               // Name of XFBIN file.
	json metadata;                  // Metadata stored to JSON.
	std::vector<char> vector_data;  // Individual byte data stored to vector.
};

// Main class for organising XFBIN files.
class File {
public:
	std::ifstream in_file;			// Input file (XFBIN, JSON, or BINARY).
	fs::path path;					// Input file's path.
	std::string extension;			// Input file's extension.
	std::string name;				// Input file's name, minus extension.
	std::vector<char> vector_data;	// Input file's data stored as a vector.
	// < consider adding const char* data >
	std::ofstream out_file;			// < consider reworking > Output file (XFBIN, JSON, or BINARY). Is used multiple times for directory creation.

	// Read file's binary data to a char vector.
	void ReadFileToVector() {
		while (in_file.peek() != EOF) {
			vector_data.push_back(in_file.get());
		}
	}

	// Write char vector data to the output file.
	void WriteVectorToFile(std::string filename) {
		out_file.open(filename, std::ios::binary);
		for (int i = 0; i < vector_data.size(); i++) {
			out_file << vector_data[i];
		}
	}

	// Load file's data and metadata into class.
	int LoadFile(fs::path argv) {
		path = argv;
		extension = path.extension().string();
		name = path.stem().string();
		if (extension == ".xfbin" && name.substr(name.size() - 4, name.size() - 1) == ".bin") {
			extension = ".bin.xfbin";
			name = path.stem().stem().string();
		}
		in_file.open(path, std::ios::binary);
		if (!in_file.is_open()) {
			return -1;
		}
		ReadFileToVector();
		return 0;
	}

	// Main function for unpacking XFBIN data.
	void Unpack_XFBIN() {
		std::string directory = name + "\\";
		std::string page_directory;
		std::string directory_name;
		std::string directory_type;
		size_t map_index_offset = 0;
		size_t extra_index_offset = 0;

		// Create base directory and extract XFBIN header information
		fs::create_directory(directory);
		out_file.open(directory + "_xfbin.json");
		json XfbinJson = Get_XFBIN_Meta(name + extension, vector_data);
		out_file << XfbinJson.dump(2);
		out_file.close();

		// Create chunk page directories
		std::ofstream page;
		int page_i = 0;
		while (vector_data.begin() + file_pos != vector_data.end()) {
			page_directory = directory + Format3Digits(page_i) + "\\";
			fs::create_directory(page_directory);

			// Create _page.json information
			json PageJson;
			bool page_end = false;
			int save_pos;
			page.open(page_directory + "_page.json");
			std::vector<chunk_struct> ChunkData;
			for (int chunk_i = 0; !page_end; chunk_i++) {
				// Get chunk information
				ChunkData.push_back(Unpack_chunkInfo(vector_data, XfbinJson, map_index_offset, chunk_i));
				PageJson.merge_patch(ChunkData[chunk_i].JsonData);
				if (ChunkData[chunk_i].Type == "nuccChunkPage") {
					page_end = true;
					PageJson.merge_patch(ExtractPageData(vector_data, XfbinJson, map_index_offset, extra_index_offset));
				} else {
					// Get chunk data
					if (ChunkData[chunk_i].Type != "nuccChunkNull") {
						// Extract to JSON.
						save_pos = file_pos; // Save current position.
						json ChunkJsonData = UnpackChunkData(vector_data, XfbinJson, map_index_offset, name, "ASBR");
						if (ChunkJsonData != nullptr) {
							out_file.open(page_directory + ChunkData[chunk_i].Name + ".json");
							out_file << ChunkJsonData.dump(2); // Unpack chunk's data via plugins.
							out_file.close();
						}

						// Extract to binary.
						file_pos = save_pos; // Reset to saved position.
						out_file.open(page_directory + ChunkData[chunk_i].Name + ".binary", std::ios::binary);
						ExtractData(vector_data, XfbinJson, out_file);
						out_file.close();
					}
				}
			}
			page << PageJson.dump(2);
			page.close();
			
			// Rename directory depending on chunks
			for (int directory_i = 0; ChunkData[directory_i].Type == "nuccChunkNull";) {
				directory_i++;
				directory_name = ChunkData[directory_i].Name;
				directory_type = ChunkData[directory_i].Type;
			}
			fs::rename(page_directory, 
				directory + "[" + Format3Digits(page_i++) + "] " + directory_name + " (" + directory_type + ")\\"
			);
		}
	}
};