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
#include <concepts>

// Short-handing.
using std::to_string;
namespace fs = std::filesystem;
using json = nlohmann::ordered_json;
#define j_get(Type) template get<Type>()

// Keep track of file data position.
extern int file_pos;

// Keep track of global parser endianness.
inline bool bigEndian;
constexpr bool IS_BIG_ENDIAN = std::endian::native == std::endian::big;

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
#define Define_Int(type, var) \
    type var; \
    Parse_Int(var, xfbin.vector_data)
#define FSkip(val) file_pos+=val

// Functions for parsing vector data.
template <std::integral T>
void Parse_Int(T& var, std::vector<char>& vector_data) {
    std::memcpy(&var, &vector_data[file_pos], sizeof(var));
    if (bigEndian == true) var = toBigEndian(var); else var = toLittleEndian(var);
    file_pos += sizeof(var);
}
std::string Parse_String(int size, std::vector<char>& vector_data);

// Format a number as 3 digits with leading 0s.
std::string Format_3_Digits(int number);

/*
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
*/
bool Does_Array_Contain(const char** arr, size_t count, std::string value);

struct plugin_metadata {
    const char** games;
	size_t games_count;
    const char** paths;
	size_t paths_count;
};

void Read_File_To_Vector(std::ifstream& file, std::vector<char>& vector_data);
void Write_Vector_To_File(std::ofstream& file, std::vector<char>& vector_data);

// Class for individual XFBIN page chunks.
class Chunk {
public:
    std::string name;               // Internal name (not literal XFBIN name).
    std::string path;               // Internal game file path (not literal XFBIN path).
    std::string type;               // e.g. "nuccChunkBinary".

	// Load metadata into JSON and file data into struct.
	void Load_Binary(uint32_t& chunk_map_offset, uint32_t& extra_map_index);
	// Parse file data to JSON file if applicable, or create binary file.
	void Unpack(fs::path directory);

	void Repack_Binary();				// Repack JSON to binary.

private:
    std::vector<char> vector_data;  // Individual byte data stored to vector.
    uint32_t size;                  // Size of chunk in bytes, used to determine the array size too. Is uint32 like in XFBIN.
    json json_data;                 // Only for binary data, represented by JSON.
	std::ofstream output_file;      // Output file, whether that be JSON or binary.
};

// Class for individual XFBIN pages.
class Page {
public:
	// Create page directory with chunk and meta data.
	void Unpack(size_t index);

private:
	uint32_t chunk_map_offset;      // Offset size from chunk map indices, used for later pages.
    uint32_t extra_map_offset;      // Offset size from extra map indices, used for later pages.
    json metadata;                  // Metadata stored to JSON.
    std::vector<Chunk> chunks;		// XFBIN chunks. Uses objects from the Chunk class.
	std::ofstream output_file;
};

// Class for XFBIN file information, whether the input is an XFBIN or an unpacked directory.
class XFBIN {
public:
	std::string name;               // Name of XFBIN file. Also the main directory name.
	std::vector<char> vector_data;  // Individual byte data stored to vector.
	json metadata;                  // Metadata stored to JSON.
	uint32_t chunk_map_offset;      // Total offset size for chunk map indices.
    uint32_t extra_map_offset;		// Total offset size for extra map indices.

	void Load_XFBIN(fs::path path) {
		// Open file into input_file.
		input_path = path;
		input_file.open(input_path, std::ios::binary);
		input_type = "xfbin";
		// Check if file succeeded in opening.
		if (!input_file.is_open()) {
			LOG_CRITICAL("XFBIN failed to open from path `" + input_path.string() + "`.");
		}

		// Set name based on filename.
		name = input_path.stem().string();

		// Read file bytes to vector_data for faster access later.
		LOG_VERBOSE("Reading file data to vector...");
		Read_File_To_Vector(input_file, vector_data);
		// Close input file as no longer needed.
		input_file.close();
		LOG_SUCCESS("XFBIN data loaded.");
	}

	void Load_JSON(fs::path path) {
		// Open file into input_file.
		input_path = path;
		input_file.open(input_path / "_xfbin.json");
		input_type = "json";

		// Check if file succeeded in opening.
		if (!input_file.is_open()) {
			LOG_CRITICAL("`_xfbin.json` not found in unpacked directory `" + input_path.string() + "`.");
		}

		metadata = json::parse(input_file);

		// Create output XFBIN.
		if (metadata["Filename"].is_null()) {
			LOG_CRITICAL("Filename could not be found in _xfbin.json");
		} else {
			name = metadata["Filename"];
			output_file.open(name + ".xfbin");
		}
	}

	void Unpack();
		void Read_Metadata(); // Reads metadata from XFBIN file and outputs to JSON.

	void Repack();

private:
	std::ifstream input_file;       // Can be XFBIN (for unpacking) or JSON (for repacking).
	fs::path input_path;
    std::string input_type;         // XFBIN or JSON depending on input.
	std::vector<Page> pages;		// XFBIN pages. Uses objects from the Page class.
	std::ofstream output_file;      // Will be the opposite of input filetype.
};
extern XFBIN xfbin;
extern std::string game;