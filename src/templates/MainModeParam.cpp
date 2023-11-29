#include "templates.h"
#include "../functions.h"

const std::string fileName = "MainModeParam";

struct panels {
    uint64_t Part;
    std::string PanelId;
    uint64_t Page;
    std::string BossPanel;
    std::string unkPanel; // Always blank
    std::string unkPanel2; // Always blank
    uint32_t GoldReward;
    uint32_t Type; // 0=Normal | 1=Extra | 2=Boss
    std::string PanelUp; // Goes to X panel when up movement key pressed
    std::string PanelDown; // Goes to X panel when down movement key pressed
    std::string PanelLeft; // Goes to X panel when left movement key pressed
    std::string PanelRight; // Goes to X panel when right movement key pressed
    uint32_t DisplayDifficulty; // Number of stars (+1) seen in UI
    uint32_t ActualDifficulty; // Actual CPU difficulty (+1) level (yes they're different fsr)
    std::string Stage;
    uint64_t unk; // Always 1
    uint64_t FirstSpeak; // 0=Player | 1=Enemy

    std::string PlayerId;
    std::string PlayerAssistId;
    std::string PlayerBattleStart;
    std::string unkPanel7; // Always blank
    std::string PlayerBattleWin;

    std::string EnemyId;
    std::string EnemyAssistId;
    std::string EnemyBattleStart;
    std::string unkPanel8; // Always blank
    std::string EnemyBattleWin;

    int32_t SpecialRule1;
    int32_t SpecialRule2;
    int32_t SpecialRule3;
    int32_t SpecialRule4;

    int32_t Mission1Condition;
    uint32_t Mission1RewardType;
    std::string Mission1Reward;
    uint64_t Mission1Gold;
    int32_t Mission2Condition;
    uint32_t Mission2RewardType;
    std::string Mission2Reward;
    uint64_t Mission2Gold;
    int32_t Mission3Condition;
    uint32_t Mission3RewardType;
    std::string Mission3Reward;
    uint64_t Mission3Gold;
    int32_t Mission4Condition;
    uint32_t Mission4RewardType;
    std::string Mission4Reward;
    uint64_t Mission4Gold;

    uint32_t unk1; // 0 for all boss panels; 2 for everything else
    uint32_t unk2; // always 0
    uint32_t unk3; // 6 for all P6 DLC panels, 2 for everything else

    uint32_t TotalPanelCount;
};
std::vector<panels> panel;

json UNPACK_MainModeParam(std::istream& input) {
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
        panel.push_back({});

        boost::pfr::for_each_field(panel[i], [&input](auto& member) { 
            if constexpr (std::is_same_v<std::remove_cvref_t<decltype(member)>,std::string>) {
                member = returnPointer(input);
            } else {
                parse(input, member);
            }
        });

        JsonData[panel[i].PanelId] = {
            {"Part", panel[i].Part},
            {"Page", panel[i].Page},
            {"Gold Reward", panel[i].GoldReward},
            {"Type", panel[i].Type},
            {"To Up", panel[i].PanelUp},
            {"To Down", panel[i].PanelDown},
            {"To Left", panel[i].PanelLeft},
            {"To Right", panel[i].PanelRight},
            {"Displayed Difficulty", panel[i].DisplayDifficulty},
            {"Actual COM Difficulty", panel[i].ActualDifficulty},
            {"Stage", panel[i].Stage},
            {"First to Speak", panel[i].FirstSpeak},
            {"Player Info", {
                {"Player ID", panel[i].PlayerId},
                {"Player Assist ID", panel[i].PlayerAssistId},
                {"Player Start Dialogue", panel[i].PlayerBattleStart},
                {"Player Win Dialogue", panel[i].PlayerBattleWin}
            }},
            {"Enemy Info", {
                {"Enemy ID", panel[i].EnemyId},
                {"Enemy Assist ID", panel[i].EnemyAssistId},
                {"Enemy Start Dialogue", panel[i].EnemyBattleStart},
                {"Enemy Win Dialogue", panel[i].EnemyBattleWin}
            }},
            {"Special Rules", {
                {"Special Rule 1", panel[i].SpecialRule1},
                {"Special Rule 2", panel[i].SpecialRule2},
                {"Special Rule 3", panel[i].SpecialRule3},
                {"Special Rule 4", panel[i].SpecialRule4}
            }},
            {"Secret Missions", {
                {"Mission 1 Condition", panel[i].Mission1Condition},
                {"Mission 1 Reward Type", panel[i].Mission1RewardType},
                {"Mission 1 Reward", panel[i].Mission1Reward},
                {"Mission 1 Gold", panel[i].Mission1Gold},
                {"Mission 2 Condition", panel[i].Mission2Condition},
                {"Mission 2 Reward Type", panel[i].Mission2RewardType},
                {"Mission 2 Reward", panel[i].Mission2Reward},
                {"Mission 2 Gold", panel[i].Mission2Gold},
                {"Mission 3 Condition", panel[i].Mission3Condition},
                {"Mission 3 Reward Type", panel[i].Mission3RewardType},
                {"Mission 3 Reward", panel[i].Mission3Reward},
                {"Mission 3 Gold", panel[i].Mission3Gold},
                {"Mission 4 Condition", panel[i].Mission4Condition},
                {"Mission 4 Reward Type", panel[i].Mission4RewardType},
                {"Mission 4 Reward", panel[i].Mission4Reward},
                {"Mission 4 Gold", panel[i].Mission4Gold},
            }},
            {"Total Panel Count", panel[i].TotalPanelCount}
        }; // JSON output
    }
    return JsonData;
}

int REPACK_MainModeParam (std::istream& input) {
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
        auto& panel_obj = json[key];
        std::string buffer;

        if (key != "Filetype" && key != "Version") {
            panel.push_back({});

            panel[i].PanelId            = key;
            panel[i].Part               = panel_obj["Part"];
            panel[i].Page               = panel_obj["Page"];

            if (panel[i].Page < 10) { 
                buffer = "0" + std::to_string(panel[i].Page); 
            } else { 
                buffer = std::to_string(panel[i].Page); 
            }
            buffer = "PANEL_" + buffer + "_08";
            panel[i].BossPanel          = buffer;

            panel[i].GoldReward         = panel_obj["Gold Reward"];
            panel[i].Type               = panel_obj["Type"];
            panel[i].PanelUp            = panel_obj["To Up"];
            panel[i].PanelDown          = panel_obj["To Down"];
            panel[i].PanelLeft          = panel_obj["To Left"];
            panel[i].PanelRight         = panel_obj["To Right"];
            panel[i].DisplayDifficulty  = panel_obj["Displayed Difficulty"];
            panel[i].ActualDifficulty   = panel_obj["Actual COM Difficulty"];
            panel[i].Stage              = panel_obj["Stage"];
            panel[i].FirstSpeak         = panel_obj["First to Speak"];

            panel[i].PlayerId           = panel_obj["Player Info"]["Player ID"];
            panel[i].PlayerAssistId     = panel_obj["Player Info"]["Player Assist ID"];
            panel[i].PlayerBattleStart  = panel_obj["Player Info"]["Player Start Dialogue"];
            panel[i].PlayerBattleWin    = panel_obj["Player Info"]["Player Win Dialogue"];

            panel[i].EnemyId            = panel_obj["Enemy Info"]["Enemy ID"];
            panel[i].EnemyAssistId      = panel_obj["Enemy Info"]["Enemy Assist ID"];
            panel[i].EnemyBattleStart   = panel_obj["Enemy Info"]["Enemy Start Dialogue"];
            panel[i].EnemyBattleWin     = panel_obj["Enemy Info"]["Enemy Win Dialogue"];

            panel[i].SpecialRule1       = panel_obj["Special Rules"]["Special Rule 1"];
            panel[i].SpecialRule2       = panel_obj["Special Rules"]["Special Rule 2"];
            panel[i].SpecialRule3       = panel_obj["Special Rules"]["Special Rule 3"];
            panel[i].SpecialRule4       = panel_obj["Special Rules"]["Special Rule 4"];

            panel[i].Mission1Condition  = panel_obj["Secret Missions"]["Mission 1 Condition"];
            panel[i].Mission1RewardType = panel_obj["Secret Missions"]["Mission 1 Reward Type"];
            panel[i].Mission1Reward     = panel_obj["Secret Missions"]["Mission 1 Reward"];
            panel[i].Mission1Gold       = panel_obj["Secret Missions"]["Mission 1 Gold"];
            panel[i].Mission2Condition  = panel_obj["Secret Missions"]["Mission 2 Condition"];
            panel[i].Mission2RewardType = panel_obj["Secret Missions"]["Mission 2 Reward Type"];
            panel[i].Mission2Reward     = panel_obj["Secret Missions"]["Mission 2 Reward"];
            panel[i].Mission2Gold       = panel_obj["Secret Missions"]["Mission 2 Gold"];
            panel[i].Mission3Condition  = panel_obj["Secret Missions"]["Mission 3 Condition"];
            panel[i].Mission3RewardType = panel_obj["Secret Missions"]["Mission 3 Reward Type"];
            panel[i].Mission3Reward     = panel_obj["Secret Missions"]["Mission 3 Reward"];
            panel[i].Mission3Gold       = panel_obj["Secret Missions"]["Mission 3 Gold"];
            panel[i].Mission4Condition  = panel_obj["Secret Missions"]["Mission 4 Condition"];
            panel[i].Mission4RewardType = panel_obj["Secret Missions"]["Mission 4 Reward Type"];
            panel[i].Mission4Reward     = panel_obj["Secret Missions"]["Mission 4 Reward"];
            panel[i].Mission4Gold       = panel_obj["Secret Missions"]["Mission 4 Gold"];

            panel[i].TotalPanelCount    = panel_obj["Total Panel Count"];
            
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
    Size += Count * 328;
    for (int m = 0; m < Count; m++) {
        boost::pfr::for_each_field(panel[m], [&Size](auto& member) { 
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
    b_pointer = (328 * Count) - 8;
    for (int j = 0; j < Count; j++) {

        binary.write((char*)&panel[j].Part, sizeof(panel[j].Part));
        writePointer(binary, b_pointer, panel[j].PanelId.size(), 16);
        binary.write((char*)&panel[j].Page, sizeof(panel[j].Page));
        writePointer(binary, b_pointer, panel[j].BossPanel.size(), 32);

        b64 = 0; // 2 unks
        binary.write((char*)&b64, sizeof(b64));
        binary.write((char*)&b64, sizeof(b64));

        binary.write((char*)&panel[j].GoldReward, sizeof(panel[j].GoldReward));
        binary.write((char*)&panel[j].Type, sizeof(panel[j].Type));
        writePointer(binary, b_pointer, panel[j].PanelUp.size(), 8);
        writePointer(binary, b_pointer, panel[j].PanelDown.size(), 8);
        writePointer(binary, b_pointer, panel[j].PanelLeft.size(), 8);
        writePointer(binary, b_pointer, panel[j].PanelRight.size(), 16);
        binary.write((char*)&panel[j].DisplayDifficulty, sizeof(panel[j].DisplayDifficulty));
        binary.write((char*)&panel[j].ActualDifficulty, sizeof(panel[j].ActualDifficulty));
        writePointer(binary, b_pointer, panel[j].Stage.size(), 24);

        b64 = 1; // unk
        binary.write((char*)&b64, sizeof(b64));

        binary.write((char*)&panel[j].FirstSpeak, sizeof(panel[j].FirstSpeak));

        writePointer(binary, b_pointer, panel[j].PlayerId.size(), 16);
        writePointer(binary, b_pointer, panel[j].PlayerAssistId.size(), 0);
        writePointer(binary, b_pointer, panel[j].PlayerBattleStart.size(), 16);

        b64 = 0; // unk
        binary.write((char*)&b64, sizeof(b64));

        writePointer(binary, b_pointer, panel[j].PlayerBattleWin.size(), 8);
        writePointer(binary, b_pointer, panel[j].EnemyId.size(), 8);
        writePointer(binary, b_pointer, panel[j].EnemyAssistId.size(), 8);
        writePointer(binary, b_pointer, panel[j].EnemyBattleStart.size(), 16);

        b64 = 0; // unk
        binary.write((char*)&b64, sizeof(b64));

        writePointer(binary, b_pointer, panel[j].EnemyBattleWin.size(), 0);

        binary.write((char*)&panel[j].SpecialRule1, sizeof(panel[j].SpecialRule1));
        binary.write((char*)&panel[j].SpecialRule2, sizeof(panel[j].SpecialRule2));
        binary.write((char*)&panel[j].SpecialRule3, sizeof(panel[j].SpecialRule3));
        binary.write((char*)&panel[j].SpecialRule4, sizeof(panel[j].SpecialRule4));

        binary.write((char*)&panel[j].Mission1Condition, sizeof(panel[j].Mission1Condition));
        binary.write((char*)&panel[j].Mission1RewardType, sizeof(panel[j].Mission1RewardType));
        writePointer(binary, b_pointer, panel[j].Mission1Reward.size(), 56);
        binary.write((char*)&panel[j].Mission1Gold, sizeof(panel[j].Mission1Gold));

        binary.write((char*)&panel[j].Mission2Condition, sizeof(panel[j].Mission2Condition));
        binary.write((char*)&panel[j].Mission2RewardType, sizeof(panel[j].Mission2RewardType));
        writePointer(binary, b_pointer, panel[j].Mission2Reward.size(), 24);
        binary.write((char*)&panel[j].Mission2Gold, sizeof(panel[j].Mission2Gold));

        binary.write((char*)&panel[j].Mission3Condition, sizeof(panel[j].Mission3Condition));
        binary.write((char*)&panel[j].Mission3RewardType, sizeof(panel[j].Mission3RewardType));
        writePointer(binary, b_pointer, panel[j].Mission3Reward.size(), 24);
        binary.write((char*)&panel[j].Mission3Gold, sizeof(panel[j].Mission3Gold));
        
        binary.write((char*)&panel[j].Mission4Condition, sizeof(panel[j].Mission4Condition));
        binary.write((char*)&panel[j].Mission4RewardType, sizeof(panel[j].Mission4RewardType));
        writePointer(binary, b_pointer, panel[j].Mission4Reward.size(), 40);
        binary.write((char*)&panel[j].Mission4Gold, sizeof(panel[j].Mission4Gold));

        if (panel[j].Type == 2) {
            b32 = 0; // unk
        } else {
            b32 = 2;
        }
        binary.write((char*)&b32, sizeof(b32));
        b32 = 0; // unk
        binary.write((char*)&b32, sizeof(b32));
        if (panel[j].Page == 12) {
            b32 = 6; // unk
        } else {
            b32 = 2;
        }
        binary.write((char*)&b32, sizeof(b32));

        binary.write((char*)&panel[j].TotalPanelCount, sizeof(panel[j].TotalPanelCount));
    }

    // write strings
    for (int j = 0; j < Count; j++) {
        boost::pfr::for_each_field(panel[j], [&binary](auto& member) { 
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
    appendXFBIN(binary, 8, 2, 7948407, 4, 0);

    binary.close();
    return 1;
}