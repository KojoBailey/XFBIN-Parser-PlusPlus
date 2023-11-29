#include "templates.h"
#include "../functions.h"

const std::string fileName = "messageInfo";

struct entries {
    std::string crc32_id;

    uint32_t unk1, unk2, unk3; // Hidden: unimportant

    std::string message;
    std::string ref_crc32;
    int16_t is_ref;

    int16_t char_id;
    int16_t cue_id;

    int16_t unk6; // Hidden: unimportant
    uint32_t unk7; // Hidden: unimportant

};
std::vector<entries> entry;

json UNPACK_messageInfo(std::istream& input) {
    // move past XFBIN fluff
    input.seekg(284);

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
        entry.push_back({});

        entry[i].crc32_id = returnHex(input);

        parse(input, entry[i].unk1);
        parse(input, entry[i].unk2);
        parse(input, entry[i].unk3);
        
        entry[i].message = returnPointer(input);

        entry[i].ref_crc32 = returnHex(input);
        parse(input, entry[i].is_ref);
        
        parse(input, entry[i].char_id);
        parse(input, entry[i].cue_id);

        parse(input, entry[i].unk6);
        parse(input, entry[i].unk7);

        JsonData[entry[i].crc32_id] = {
            {"message", entry[i].message},
            {"is_ref", entry[i].is_ref},
            {"ref_crc32", entry[i].ref_crc32},
            {"char_id", entry[i].char_id},
            {"cue_id", entry[i].cue_id}
        }; // JSON output
    }
    return JsonData;
}

int REPACK_messageInfo (std::istream& input) {
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
        auto& entry_obj = json[key];

        if (key != "Filetype" && key != "Version") {
            entry.push_back({});

            if (key.size() != 8) {
                sendError("Detected a CRC32 ID without 8 characters.\n");
                EndProgram(); return 0;
            }
            entry[i].crc32_id = key;
            entry[i].message = entry_obj["message"];
            entry[i].ref_crc32 = entry_obj["ref_crc32"];
            entry[i].is_ref = entry_obj["is_ref"];
            entry[i].char_id = entry_obj["char_id"];
            entry[i].cue_id = entry_obj["cue_id"];
            
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
    Size += Count * 40;
    for (int m = 0; m < Count; m++) {
        increaseSize(Size, entry[m].message);
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
    b32 = toBigEndian(7956736);
    binary.write((char*)&b32, sizeof(b32));

    // write start-of-data stuff
    binary.write((char*)&Size, sizeof(Size));
    binary.write((char*)&Version, sizeof(Version));
    binary.write((char*)&Count, sizeof(Count));
    binary.write((char*)&Pointer, sizeof(Pointer));

    // write numerical data
    b_pointer = (40 * Count) - 16;
    for (int j = 0; j < Count; j++) {
        // crc32_id
        b32 = std::stoul(entry[j].crc32_id.c_str(), nullptr, 16);
        b32 = toBigEndian(b32);
        binary.write((char*)&b32, sizeof(b32));

        // blank bytes
        b32 = 0x0000;
        b64 = 0x0000;
        binary.write((char*)&b64, sizeof(b64));
        binary.write((char*)&b32, sizeof(b32));

        // message pointer
        writePointer(binary, b_pointer, entry[j].message.size(), 40);

        // ref_crc32
        if (entry[j].ref_crc32 == "") {
            b32 = 0x0000;
        } else {
            b32 = std::stoul(entry[j].ref_crc32.c_str(), nullptr, 16);
            b32 = toBigEndian(b32);
        }
        binary.write((char*)&b32, sizeof(b32));
        binary.write((char*)&entry[j].is_ref, sizeof(entry[j].is_ref));

        // char_id
        binary.write((char*)&entry[j].char_id, sizeof(entry[j].char_id));
        // cue_id
        binary.write((char*)&entry[j].cue_id, sizeof(entry[j].cue_id));

        // blank bytes
        b16 = 0xFFFF;
        b32 = 0x0000;
        binary.write((char*)&b16, sizeof(b16));
        binary.write((char*)&b32, sizeof(b32));
    }

    // write strings
    for (int j = 0; j < Count; j++) {
        writeString(binary, entry[j].message);
    }

    // make file size a multiple of 4
    while (saveSize % 4 != 0) {
        binary.put(0x00);
        saveSize++;
    }
    
    // add end of XFBIN stuff
    appendXFBIN(binary, 8, 2, 7951735, 4, 0);

    binary.close();
    return 1;
}