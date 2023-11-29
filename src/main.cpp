// g++ src/main.cpp src/functions.cpp src/templates/messageInfo.cpp src/templates/MainModeParam.cpp -o main.exe -std=c++20 -static -static-libgcc -static-libstdc++

#include "functions.h"
#include "templates/templates.h"

namespace fs = std::filesystem;

int runFunctions(std::ifstream& input, std::ofstream& output, std::string fileType, std::string fileExtension, std::string outputFileExtension) {
    char xfbinCheck[1];
    std::unordered_map<std::string, json(*)(std::istream&)> UNPACK_;
    std::unordered_map<std::string, int(*)(std::istream&)> REPACK_;
    UNPACK_["messageInfo"] = UNPACK_messageInfo; REPACK_["messageInfo"] = REPACK_messageInfo;
    UNPACK_["GuideCharParam"] = UNPACK_GuideCharParam; REPACK_["GuideCharParam"] = REPACK_GuideCharParam;
    UNPACK_["MainModeParam"] = UNPACK_MainModeParam; REPACK_["MainModeParam"] = REPACK_MainModeParam;
    UNPACK_["MainModeParam"] = UNPACK_MainModeParam; REPACK_["MainModeParam"] = REPACK_MainModeParam;

    std::cout << "> Filetype \"" + fileType + fileExtension + "\" detected. File will output as \"" + fileType + outputFileExtension + "\"\n";
    std::cout << "> Parsing in progress...\n";
    if (fileExtension == ".bin.xfbin") {
        input.read(xfbinCheck, sizeof(xfbinCheck));
        if (xfbinCheck[0] != 'N') {
            sendError("Invalid XFBIN.\n");
            return -1;
        }
        output.open(fileType + outputFileExtension);
        output << UnpackBinary(input, fileType).dump(2); // UNPACK_[fileType](input).dump(2);
        return 1;
    } else if (fileExtension == ".json") {
        int returnCheck = REPACK_[fileType](input);
        if (returnCheck == 0) { return -1; } else { return 1; }
    }

    sendError("Could not match file name \"" + fileType + fileExtension + "\" to a file type.\n");
    std::cout << "Which type would you like to input?\n> (e.g. messageInfo)\n";
    std::cin >> fileType;
    return 0;
}

int main(int argc, char* argv[]) {
    std::string fileType;

    const int supportedFilesCount = 3;
    std::string supportedFiles[supportedFilesCount] = {"messageInfo", "GuideCharParam", "MainModeParam"};

    std::cout << "\033[1;35m" << "JoJo's Bizarre Adventure: All-Star Battle R - XFBIN-JSON Parser & Merger" << "\033[0m\n\n";
    std::cout << "> Current supported ASBR file types are:\n";
    for (int i = 0; i < supportedFilesCount; i++) {
        std::cout << "\t- \033[1;33m" << supportedFiles[i] << "\033[0m\n";
    }
    std::cout << "\n\n";

    if (argv[1]) {                                               // serialisation
        fs::path filePath = argv[1];
        std::string fileExtension = filePath.extension().string();
        if (fileExtension == ".xfbin") {
            fileExtension = ".bin.xfbin";
        }

        std::ifstream input;
        std::string outputFileExtension;
        if (fileExtension == ".bin.xfbin") {
            input.open(filePath, std::ios::binary);
            outputFileExtension = ".json";
            fileType = filePath.stem().stem().string();
        } else if (fileExtension == ".json") {
            input.open(filePath);
            outputFileExtension = ".bin.xfbin";
            fileType = filePath.stem().string();
        } else {
            sendError("Invalid file extension. Must be XFBIN or JSON.\n");
            EndProgram(); return 0;
        }

        std::ofstream output;

        int parsing = 0;
        while (parsing == 0) {
            parsing = runFunctions(input, output, fileType, fileExtension, outputFileExtension);
            if (parsing == -1) { EndProgram(); return 0; }
        }

        std::cout << "> \033[32mParse successful!\033[0m\n";
    } else {                                                     // merging
        std::cout << "> To \033[1;36mparse\033[0m a file to JSON or back, simply close this EXE and drag the file you want onto it." << std::endl;
        std::cout << std::endl << "\033[1;35m" << "**********  OR  **********" << "\033[0m" << "\n\n";
        std::cout << "> To \033[1;36mmerge\033[0m JSON files, input the type of file you want to merge below." << std::endl;
        std::cout << "File type (e.g. \033[1;33mmessageInfo\033[0m): ";
        std::cin >> fileType;
        fs::create_directory("merging");
        fs::path mergeDirectory = "merging\\" + fileType;
        fs::create_directory(mergeDirectory);

        struct mergeFileInfo {
            std::string name;
            int priority;
        };
        std::vector<mergeFileInfo> mergeFile;

        std::ifstream mergeInfo;

        std::cout << "\n> Merging JSON files..." << std::endl;

        // get list of JSON files to be merged
        int i = 0;
        std::string buffer;
        for (auto& x : fs::directory_iterator(mergeDirectory)) {
            if (x.path().extension() == ".json") {
                mergeFile.push_back({"", 0});
                mergeFile[i].name = x.path().stem().string();
                mergeInfo.open(mergeDirectory.string() + "\\" + mergeFile[i].name + ".txt");
                getline(mergeInfo, buffer);
                mergeInfo.close();
                mergeFile[i].priority = stoi(buffer);
                i++;
            }
        }

        // sort in ascending priority
        std::sort(mergeFile.begin(), mergeFile.end(), [](const mergeFileInfo& a, const mergeFileInfo& b) {
            return a.priority < b.priority;
        });

        // get base file
        std::ifstream baseFile("data\\" + fileType + ".json");
        json j_base = json::parse(baseFile);
        baseFile.close();

        // merge and output
        std::ifstream mergeJSON;
        std::ofstream mergeOutput(fileType + ".json");
        json j_merge;
        for (int i = 0; i < mergeFile.size(); i++) {
            std::cout << "> Found file \033[1;33m" << mergeFile[i].name << "\033[0m with priority \033[1;33m" << mergeFile[i].priority << "\033[0m.\n";
            mergeJSON.open(mergeDirectory.string() + "\\" + mergeFile[i].name + ".json");
            j_merge = json::parse(mergeJSON);
            mergeJSON.close();
            j_base.merge_patch(j_merge);
        }
        mergeOutput << j_base.dump(2);
        mergeOutput.close();
        std::cout << "\n> \033[1;33m" + fileType + "\033[32m JSON merged succesfully!\033[0m" << std::endl;
    }

    EndProgram();
}