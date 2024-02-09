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

#define DefInt(type, var) \
    type var; \
    parseInt(var, data, pos)

#define Skip(val) pos+=val

#define str(x) std::to_string(x)

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
void parseInt(T& var, std::vector<char>& data, int& pos) {
    std::memcpy(&var, &data[pos], sizeof(var));
    if (bigEndian == true) var = toBigEndian(var); else var = toLittleEndian(var);
    pos += sizeof(var);
}
std::string parseChar(int size, std::vector<char>& data, int& pos);

json Unpack_xfbin(std::string name, std::vector<char>& data);
void ExtractData(std::vector<char>& data, json XfbinJson, std::ofstream& out_file);
json Unpack_ASBR_sndcmnparam(std::vector<char>& data, json XfbinJson, int index);

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
		fs::create_directory(directory);
		out_file.open(directory + "_XFBIN.json");
		json XfbinJson = Unpack_xfbin(name + extension, data);
		out_file << XfbinJson.dump(2);
		out_file.close();

		directory += "[000] " + name + " (nuccChunkBinary)\\";
		fs::create_directory(directory);
		out_file.open(directory + name + ".json");
		if (name == "sndcmnparam") {
            out_file << Unpack_ASBR_sndcmnparam(data, XfbinJson, 1).dump(2);
		}
		out_file.close();
		out_file.open(directory + name + ".binary", std::ios::binary);
		ExtractData(data, XfbinJson, out_file);
		out_file.close();
	}
};
