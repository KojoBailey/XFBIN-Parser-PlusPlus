#include "templates.h"
#include "../functions.h"

json UnpackBinary(std::istream& input, std::string fileType) {
    // Move past XFBIN fluff - Needs to be replaced later with actual XFBIN parsing
    input.seekg(276);

    // An easy way to know the size of each data type from strings
    std::unordered_map<std::string, int> varSize;
    varSize["uint8"] = 1, varSize["u8"] = 1;
    varSize["uint16"] = 2, varSize["u16"] = 2;
    varSize["uint32"] = 4, varSize["u32"] = 4;
    varSize["uint64"] = 8, varSize["u64"] = 8;
    varSize["int8"] = 1, varSize["i8"] = 1;
    varSize["int16"] = 2, varSize["i16"] = 2;
    varSize["int32"] = 4, varSize["i32"] = 4;
    varSize["int64"] = 8, varSize["i64"] = 8;
    varSize["half_float"] = 2, varSize["hf"] = 2;
    varSize["float"] = 4, varSize["f"] = 4;
    varSize["double"] = 8, varSize["d"] = 8;

    json JsonData; // Where the JSON data will be stored
    JsonData["Filetype"] = fileType; // So the parser can know what filetype it's dealing with automatically

    // Try to open matching CCBT
    std::ifstream ccbt(fileType + ".ccbt");
    if (!ccbt.is_open()) {
        sendError("Could not locate \"" + fileType + ".ccbt\".");
        return 0; // !! need to add error functionality to main.cpp
    }

    // Variables to be used in the CCBT parsing
    std::unordered_map<int, std::tuple<std::string, std::string, std::string>> var;
    std::string varType, varName, varValue;
    std::unordered_map<std::string, int> temp_var; // For temporary variables
    uint8_t buffer8;
    uint16_t buffer16;
    uint32_t buffer32;
    uint64_t buffer64;

    // To keep track of the curly braces/brackets
    std::stack<int> curly_braces;

    // Parse CCBT
    for (int i = 1; !ccbt.eof(); i++) {
        // Output data to console (testing purposes for now)
        if (i-1 > 0) {
            std::cout << "Order: " << i-1 << "\nvarType: " << std::get<0>(var[i-1]) << "\nvarName: " << std::get<1>(var[i-1]) << "\n\n";
        }

        // Check the data type or whether it's a function/key-word
        varType = readCCBT(ccbt);

        // If function detected
        if (ccbt.peek() == ';') {
            if (varType == "BigEndian()") {
                BigEndian();
            } else if (varType == "LittleEndian()") {
                LittleEndian();
            }
            ccbt.ignore(1);
            var.insert(std::make_pair(i, createTuple("Function", varType, "")));
            continue; // Skips the rest of this loop iteration
        }

        // If key word detected
        if (varType == "for") {
            varName = readCCBT(ccbt, "for init var"); // Get initalising variable name
            temp_var[varName] = std::stoi(readCCBT(ccbt, "for init val")); // Get init var value (int)
            varValue = readCCBT(ccbt, "for rest"); // Get rest of for loop parameter data

            var.insert(std::make_pair(i, createTuple("for {", varName, varValue)));
            std::cout << "For: " << varName << " = " << temp_var[varName] << "\nData: " << varValue << "\n\n";
            curly_braces.push(i);
            continue;
        }

        // If "}" detected
        if (varType == "}") {
            if (curly_braces.empty()) {
                sendError("Isolated curly brace ( \"}\" ) detected and ignored.");
            } else {
                varValue = std::to_string(curly_braces.top());
                var.insert(std::make_pair(i, createTuple("for }", "", varValue)));
                std::cout << "End: " <<  varValue << "\n\n";
                curly_braces.pop();
            }
        }


        // Check the variable name
        varName = readCCBT(ccbt, "name");
        varValue = "";
        // if (any_of(varType, "string", "str")) {
        //     varValue = returnPointer(input);
        // } else {
        //     if (varSize[varType] == 1) {
        //         parse(input, buffer8, varSize[varType]);
        //         varValue = std::to_string(buffer8);
        //     } else if (varSize[varType] == 2) {
        //         parse(input, buffer16, varSize[varType]);
        //         varValue = std::to_string(buffer16);
        //     } else if (varSize[varType] == 4) {
        //         parse(input, buffer32, varSize[varType]);
        //         varValue = std::to_string(buffer32);
        //     } else if (varSize[varType] == 8) {
        //         parse(input, buffer64, varSize[varType]);
        //         varValue = std::to_string(buffer64);
        //     }
        // }
        ccbt.ignore(1);

        // Store the data for easy access later
        var.insert(std::make_pair(i, createTuple(varType, varName, varValue)));
    }
    std::cout << "Test complete\n"; 
    system("pause");
    return 0;
}