#include "templates.h"
#include "../functions.h"

const std::string fileName = "GuideCharParam";

struct events {
    std::string Call;
    std::string Character;

    std::string NameID1, AudioID1;
    std::string NameID2, AudioID2;
    std::string NameID3, AudioID3;
    std::string NameID4, AudioID4;
    std::string NameID5, AudioID5;
};
std::vector<events> event;

json UNPACK_GuideCharParam(std::istream& input) {
    // move past XFBIN fluff
    input.seekg(276);

    json JsonData; // Where the JSON data will be stored
    JsonData["Filetype"] = fileName; // So the parser can know what filetype it's dealing with automatically

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

    JsonData["Version"] = Version;

    for (int i = 0; i < Count; i++) {
        event.push_back({});

        boost::pfr::for_each_field(event[i], [&input](auto& member) { 
            if constexpr (std::is_same_v<std::remove_cvref_t<decltype(member)>,std::string>) {
                member = returnPointer(input);
            }
        });

        JsonData[event[i].Call] = {
            {"Character", event[i].Character},
            {"Action 1", {
                {"Name ID", event[i].NameID1},
                {"Audio ID", event[i].AudioID1}
            }},
            {"Action 2", {
                {"Name ID", event[i].NameID2},
                {"Audio ID", event[i].AudioID2}
            }},
            {"Action 3", {
                {"Name ID", event[i].NameID3},
                {"Audio ID", event[i].AudioID3}
            }},
            {"Action 4", {
                {"Name ID", event[i].NameID4},
                {"Audio ID", event[i].AudioID4}
            }}
        }; // JSON output
    }
    return JsonData;
}

int REPACK_GuideCharParam (std::istream& input) {
    json json = json::parse(input);

    std::ofstream binary(fileName + ".bin.xfbin", std::ios::binary);

    // get and write XFBIN structure data
    std::ifstream xfbinData("data/" + fileName + ".xfbin.dat", std::ios::binary);
    XFBIN(xfbinData, binary);
    xfbinData.close();

    int i = 0;
    std::string key;
    for (auto& x : json.items()) {
        key = x.key();
        auto& event_obj = json[key];
        std::string buffer;

        if (key != "Filetype" && key != "Version") {
            event.push_back({});

            event[i].Call               = key;
            event[i].Character          = event_obj["Character"];

            event[i].NameID1            = event_obj["Action 1"]["Name ID"];
            event[i].AudioID1           = event_obj["Action 1"]["Audio ID"];
            event[i].NameID2            = event_obj["Action 2"]["Name ID"];
            event[i].AudioID2           = event_obj["Action 2"]["Audio ID"];
            event[i].NameID3            = event_obj["Action 3"]["Name ID"];
            event[i].AudioID3           = event_obj["Action 3"]["Audio ID"];
            event[i].NameID4            = event_obj["Action 4"]["Name ID"];
            event[i].AudioID4           = event_obj["Action 4"]["Audio ID"];
            event[i].NameID5            = "";
            event[i].AudioID5           = "";
            // NameID5 and AudioID5 don't actually work
            
            i++;
        }
    }
    uint16_t b16; // buffer16
    uint32_t b32; // buffer32
    uint64_t b64; // buffer64
    uint64_t b_pointer; // buffer pointer

    uint32_t Size = 20; // size of pre-entries stuff
    uint32_t Version = json["Version"];
    uint32_t Count = i; // # of entries
    uint64_t Pointer = 8;

    // calculate final filesize
    Size += Count * 96;
    for (int m = 0; m < Count; m++) {
        boost::pfr::for_each_field(event[m], [&Size](auto& member) { 
            if constexpr (std::is_same_v<std::remove_cvref_t<decltype(member)>,std::string>) {
                increaseSize(Size, member);
            }
        });
    }
    int saveSize = Size; // to be used later
    while (Size % 4 != 0) {
        Size++;
    }
    Size = toBigEndian(Size - 4);

    // more XFBIN writing
    uint32_t TotalSize = toBigEndian(toBigEndian(Size) + 4);
    binary.write((char*)&TotalSize, sizeof(TotalSize));
    b32 = toBigEndian(1);
    binary.write((char*)&b32, sizeof(b32));
    b32 = toBigEndian(7936256);
    binary.write((char*)&b32, sizeof(b32));

    // write start-of-data stuff
    binary.write((char*)&Size, sizeof(Size));
    binary.write((char*)&Version, sizeof(Version));
    binary.write((char*)&Count, sizeof(Count));
    binary.write((char*)&Pointer, sizeof(Pointer));

    // write numerical data
    b_pointer = (96 * Count);
    for (int j = 0; j < Count; j++) {
        boost::pfr::for_each_field(event[j], [&binary, &b_pointer](auto& member) { 
            if constexpr (std::is_same_v<std::remove_cvref_t<decltype(member)>,std::string>) {
                writePointer(binary, b_pointer, member.size(), 8);
            }
        });
    }

    // write strings
    for (int j = 0; j < Count; j++) {
        boost::pfr::for_each_field(event[j], [&binary](auto& member) { 
            if constexpr (std::is_same_v<std::remove_cvref_t<decltype(member)>,std::string>) {
                writeString(binary, member);
            }
        });
    }

    // make file size a multiple of 4
    while (saveSize % 4 != 0) {
        binary.put(0x00);
        saveSize++;
    }
    
    // add end of XFBIN stuff
    appendXFBIN(binary, 8, 2, 7978871, 4, 0);

    binary.close();
    return 1;
}