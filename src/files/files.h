#pragma once

#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>
#include <nlohmann/json.hpp>
#include <cstdint>
#include <unordered_map>
#include <bit>

namespace fs = std::filesystem;
using json = nlohmann::ordered_json;

inline bool bigEndian;
#define IS_BIG_ENDIAN (std::endian::native == std::endian::big)

// function macros
#define DefineVectorInt(type, var) \
    type var; \
    parseVectorInt(var, data)
#define Skip(val) Pos+=val
#define str(x) std::to_string(x)
#define j_get(Type) template get<Type>()

// Keep track of file data position.
extern int Pos;

void BigEndian();
void LittleEndian();

template <std::integral T>
T toBigEndian(T value) {
    if (!IS_BIG_ENDIAN) return std::byteswap(value); else return value;
}
template <std::integral T>
T toLittleEndian(T value) {
    if (IS_BIG_ENDIAN) return std::byteswap(value); else return value;
}

template <std::integral T>
void parseVectorInt(T& var, std::vector<char>& data) {
    std::memcpy(&var, &data[Pos], sizeof(var));
    if (bigEndian == true) var = toBigEndian(var); else var = toLittleEndian(var);
    Pos += sizeof(var);
}
std::string parseVectorChar(int size, std::vector<char>& data);

std::string Format3Digits(int number);

json Unpack_xfbin(std::string name, std::vector<char>& data);
struct chunk_struct {
	json JsonData;
	std::string Size;
	std::string Type;
	std::string Path;
	std::string Name;
};
chunk_struct Unpack_chunkInfo(std::vector<char>& data, json& XfbinJson, size_t& map_index_offset, int chunk_i);
json ExtractPageData(std::vector<char>& data, json& XfbinJson, size_t& map_index_offset, size_t& extra_index_offset);
void ExtractData(std::vector<char>& data, json XfbinJson, std::ofstream& out_file);
json UnpackChunkData(std::vector<char>& data, json& XfbinJson, size_t index, std::string chunk_path, std::string game);
struct plugin_metadata {
    const char** games;
	size_t games_count;
    const char** paths;
	size_t paths_count;
};

class File {
public:
	std::ifstream in_file;
	fs::path path;
	std::string extension;
	std::string name;
	std::vector<char> data;
	std::ofstream out_file;

	void ReadFileToVector() {
		while (in_file.peek() != EOF) {
			data.push_back(in_file.get());
		}
	}

	void WriteVectorToFile(std::string filename) {
		out_file.open(filename, std::ios::binary);
		for (int i = 0; i < data.size(); i++) {
			out_file << data[i];
		}
	}

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

	void Unpack() {
		std::string directory = name + "\\";
		std::string page_directory;
		std::string directory_name;
		std::string directory_type;
		size_t map_index_offset = 0;
		size_t extra_index_offset = 0;

		// Create base directory and extract XFBIN header information
		fs::create_directory(directory);
		out_file.open(directory + "_xfbin.json");
		json XfbinJson = Unpack_xfbin(name + extension, data);
		out_file << XfbinJson.dump(2);
		out_file.close();

		// Create chunk page directories
		std::ofstream page;
		int page_i = 0;
		while (data.begin() + Pos != data.end()) {
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
				ChunkData.push_back(Unpack_chunkInfo(data, XfbinJson, map_index_offset, chunk_i));
				PageJson.merge_patch(ChunkData[chunk_i].JsonData);
				if (ChunkData[chunk_i].Type == "nuccChunkPage") {
					page_end = true;
					PageJson.merge_patch(ExtractPageData(data, XfbinJson, map_index_offset, extra_index_offset));
				} else {
					// Get chunk data
					if (ChunkData[chunk_i].Type != "nuccChunkNull") {
						// Extract to JSON.
						save_pos = Pos; // Save current position.
						out_file.open(page_directory + ChunkData[chunk_i].Name + ".json");
						out_file << UnpackChunkData(data, XfbinJson, map_index_offset, name, "ASBR").dump(2); // Unpack chunk's data via plugins.
						out_file.close();

						// Extract to binary.
						Pos = save_pos; // Reset to saved position.
						out_file.open(page_directory + ChunkData[chunk_i].Name + ".binary", std::ios::binary);
						ExtractData(data, XfbinJson, out_file);
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