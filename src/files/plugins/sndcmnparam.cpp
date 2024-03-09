// sndcmnparam.dll

#include "dll.h"
#include "../../logger.h"

plugin_metadata get_plugin_metadata() {
    // Supported games.
    std::vector<std::string> games = {"ASB", "ASBR"};

    // Supported internal file paths.
    std::vector<std::string> paths;
    std::vector<std::string> regex_paths = {
        "cmnparam/bin/XXX/battle.bin",
        "cmnparam/bin/XXX/commse0.bin",
        "cmnparam/bin/XXX/gimmick.bin",
        "cmnparam/bin/XXX/player.bin",
        "cmnparam/bin/XXX/sd01a_mob.bin",
        "cmnparam/bin/XXX/sd02a_mob.bin",
        "cmnparam/bin/XXX/sd03b_mob.bin",
        "cmnparam/bin/XXX/sd04a_mob.bin",
        "cmnparam/bin/XXX/sd04b_mob.bin",
        "cmnparam/bin/XXX/sd05a_mob.bin",
        "cmnparam/bin/XXX/sd06a_mob.bin",
        "cmnparam/bin/XXX/sd06b_mob.bin",
        "cmnparam/bin/XXX/sd07a_mob.bin",
        "cmnparam/bin/XXX/sd08a_mob.bin"
    };
    std::vector<std::string> supported_versions = {"100", "110", "130", "140", "150", "160", "170", "200", "210", "220", "230"};
    Regex_Add_Paths(paths, regex_paths, supported_versions);

    return create_metadata(games, paths);
}

const char* unpack(const char* data) {
    // Define globals.
    chunk_data = data;
    chunk_pos = 0;

    /* DEFINE VARIABLES */
        auto size = Parse<uint32>(BIG); // BIG → Total file size.
        auto count = Parse<uint16>(LITTLE); // LITTLE → Entry count.
        LOG_DEBUG(str(count));

        std::vector<string> entries; // List of audio name IDs.
        for (auto i = 0; i < count; i++) {
            entries.push_back(Parse<string>(32));
        }
    
    /* DEFINE JSON */
        json output_json = {
            {"Entry Count", count}
        };
        for (auto i = 0; i < count; i++) {
            LOG_DEBUG(entries[i]);
            output_json["Entry " + str(i + 1)] = entries[i];
        }

    // Allocate memory for string and return.
    return JSON_to_C_Str(output_json);
}