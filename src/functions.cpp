#include "functions.h"

void sendError (std::string message) {
    std::cerr << "\033[31m" << "> ERROR: " << message << "\033[0m";
}

void EndProgram() {
    std::cout << "\n\n";
    system("pause");
}

void LittleEndian() {
    littleEndian = true;
}
void BigEndian() {
    littleEndian = false;
}

std::string swapBytes(std::string input) {
    int size = input.size() / 2;
    std::string output;
    for (int i = size; i > 0; i--) {
        output += input.at(i * 2 - 2);
        output += input.at(i * 2 - 1);
    }
    return output;
}

std::string parseStr(std::istream& file, char delimiter) {
    char buffer;
    std::string output;
    while (file.peek() != delimiter) {
        file.read((char*)&buffer, 1);
        output += buffer;
    }
    return output;
} // default delimiter is '\0' (for reading binaries)

std::tuple<std::string, std::string, std::string> createTuple(std::string varType, std::string varName, std::string varValue) {
    return std::make_tuple(varType, varName, varValue);
}

void XFBIN(std::istream& input, std::ostream& output) {
    char xfbinByte;
    input.seekg(0, std::ios::end);
    int size = input.tellg();
    input.seekg(0, std::ios::beg);
    for (int i = 0; i < size; i++) {
        input.read((&xfbinByte), 1);
        output.put(xfbinByte);
    }
}

std::string returnPointer(std::istream& input) {
    std::string Value;
    uint64_t Pointer; parse(input, Pointer);
    if (Pointer == 0) {
        Value = "";
    } else {
        int Pos = input.tellg();
        input.seekg(Pos - 8 + Pointer); // Jumps forward by Pointer bytes
        Value = parseStr(input);
        input.seekg(Pos); // Go back to before jump
    }
    return Value;
}
std::string returnHex(std::istream& input) {
    uint32_t buffer;
    parse(input, buffer);
    return swapBytes(std::format("{:08x}", buffer));
}

void writePointer(std::ofstream& binary, uint64_t& pointer, int size, int offset) {
    if (size > 0) {
        binary.write((char*)&pointer, sizeof(pointer));
        pointer += size + 1;
    } else {
        uint64_t buffer = 0;
        binary.write((char*)&buffer, sizeof(buffer));
    }
    pointer -= offset;
}

void writeString(std::ofstream& binary, std::string string) {
    if (string != "") {
        binary.write(string.c_str(), string.size());
        binary.put(0x00);
    }
}

void increaseSize(uint32_t& Size, std::string Value) {
    if (Value.size() > 0) {
        Size += Value.size() + 1;
    }
}

void appendXFBIN(std::ofstream& file, int a, int b, int c, int d, int e) {
    uint32_t b32;
    b32 = toBigEndian(a);
    file.write((char*)&b32, sizeof(b32));
    b32 = toBigEndian(b);
    file.write((char*)&b32, sizeof(b32));
    b32 = toBigEndian(c);
    file.write((char*)&b32, sizeof(b32));
    b32 = toBigEndian(d);
    file.write((char*)&b32, sizeof(b32));
    b32 = toBigEndian(e);
    file.write((char*)&b32, sizeof(b32));
}

std::string readCCBT(std::istream& file, std::string type) {
    char buffer;
    std::string output;
    while (any_of(file.peek(), ' ', '\n')) {
        file.ignore(1);
    }
    if (file.peek() == '/') {
        file.ignore(1);
        if (file.peek() == '/') {
            while(file.peek() != '\n') {
                file.ignore(1);
            }
            return readCCBT(file, type);
        }
    } else if (file.peek() == '}') {
        file.ignore(1);
        return "}";
    }
    if (type == "name") {
        while (file.peek() != ';') {
            file.read((char*)&buffer, 1);
            output += buffer;
        }
    } else if (type == "for init var") {
        while (file.peek() != '(') {
            file.ignore(1);
        }
        file.ignore(1);
        while (file.peek() != '=') {
            file.read((char*)&buffer, 1);
            output += buffer;
        }
        file.ignore(1);
    } else if (type == "for init val") {
        while (file.peek() != ';') {
            file.read((char*)&buffer, 1);
            output += buffer;
        }
        file.ignore(1);
    } else if (type == "for rest") {
        while (file.peek() != ')') {
            file.read((char*)&buffer, 1);
            output += buffer;
        }
        while (file.peek() != '{') {
            file.ignore(1);
        }
        file.ignore(1);
    } else {
        while (!any_of(file.peek(), ' ', ';')) {
            file.read((char*)&buffer, 1);
            output += buffer;
        }
    }

    // Remove excess spaces and skip accordingly
    if (output[output.size() - 1] == ' ') {
        output.pop_back();
        file.ignore(1);
    }
    if (output[0] == ' ') {
        output = output.substr(1, output.size());
    }

    return output;
}