#include "files.h"

// Keep track of global parser position in data.
int file_pos = 0;

// Set global parser endianness.
void BigEndian() {
    bigEndian = true;
}
void LittleEndian() {
    bigEndian = false;
}

// Parsing vector char data.
std::string Parse_String(int size, std::vector<char>& data) {
    std::string buffer = "";
    if (size > 0) {
        for (int i = 0; i < size; i++) {
            if (data[file_pos] != '\0') {
                buffer += data[file_pos];
            }
            file_pos++;
        }
    } else {
        for (int i = 0; data[file_pos] != '\0'; i++) {
            buffer += data[file_pos];
            file_pos++;
        }
        file_pos++;
    }
    return buffer;
}

// Format a number as 3 digits with leading 0s.
std::string Format3Digits(int number) {
    if (number < 10)
        return "00" + std::to_string(number);
    else if (number < 100)
        return "0" + std::to_string(number);
    else
        return std::to_string(number);
}

void Read_File_To_Vector(std::ifstream& file, std::vector<char>& vector_data) {
	vector_data.clear();
	while (file.peek() != EOF) {
		vector_data.push_back(file.get());
	}
}