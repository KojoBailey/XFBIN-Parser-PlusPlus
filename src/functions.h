#include <iostream>
#include <fstream>
#include <filesystem>
#include <format>
#include <string>
#include <vector>
#include <cstdint>
#include <bit>
#include <algorithm>
#include <cmath>
#include <tuple>
#include <unordered_map>
#include <stack>
#include <boost/pfr.hpp>

inline bool littleEndian;

#define IS_BIG_ENDIAN (std::endian::native == std::endian::big)

void sendError (std::string message);

void EndProgram();

template <typename T>
T toBigEndian(T value) {
    if (!IS_BIG_ENDIAN) return _byteswap_ulong(value); else return value;
}
template <typename T>
T toLittleEndian(T value) {
    if (IS_BIG_ENDIAN) return _byteswap_ulong(value); else return value;
}

void LittleEndian();
void BigEndian();

template<typename T, typename ... Cs>
constexpr bool any_of(const T& cmp, const Cs& ... others) {
  return ((cmp == others) || ...);
}

template <typename T>
void parse(std::istream& file, T& param, int size = 0) {
    if (size == 0) {
        size = sizeof(param);
    }
    file.read((char*)&param, size);
    if (size > 0) {
        if (littleEndian == false) {
            param = toBigEndian(param);
        } else {
            param = toLittleEndian(param);
        }
    }
}

std::string parseStr(std::istream& file, char delimiter = '\0'); // gets a pointer's corresponding string

std::string swapBytes(std::string input);

void XFBIN(std::istream& input, std::ostream& output);

std::string returnPointer(std::istream& input); // finds a pointer and gets the corresponding string
std::string returnHex(std::istream& input); // gets the raw hex bytes of data

void writePointer(std::ofstream& binary, uint64_t& pointer, int size, int offset);

void writeString(std::ofstream& binary, std::string string);

void increaseSize(uint32_t& Size, std::string Value);

void appendXFBIN(std::ofstream& file, int a, int b, int c, int d, int e);

std::tuple<std::string, std::string, std::string> createTuple(std::string varType, std::string varName, std::string varValue);

std::string readCCBT(std::istream& file, std::string type = "default");