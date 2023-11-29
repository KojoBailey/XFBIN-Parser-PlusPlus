#include "templates.h"
#include "../functions.h"

json UNPACK_MainModeParam(std::istream& input) {
    // move past XFBIN fluff
    input.seekg(276);

    json JsonData; // Where the JSON data will be stored
    JsonData["Filetype"] = "MainModeParam"; // So the parser can know what filetype it's dealing with automatically

    BigEndian();
    uint32_t Size;
    parse(input, Size); // Hidden: modified automatically
    Size = toBigEndian(Size);
    LittleEndian();
    uint32_t Version, Count;
    uint64_t FirstPointer;
    parse(input, Version);
    parse(input, Count); // Hidden: modified automatically
    parse(input, FirstPointer); // Hidden: unimportant
}