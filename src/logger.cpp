#include "logger.h"

#include <filesystem>
#include <iostream>
#include <string>
#include <fstream>
#include <limits>
#include <toml++/toml.hpp>

namespace fs = std::filesystem;

void End_Program(int exitCode) {
    std::cout << "Press Enter to exit...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::exit(exitCode);
}

// Define default project name.
std::string THIS_PROJECT_NAME = "Xfbin Parser++";
#define PROJECT_NAME "Xfbin Parser++"

// Log file.
static std::ofstream log_file;

// Load configuration data.
log_configs log_config;
void Load_Log_Config() {
    // Create config file if not already existing.
    if (!fs::exists("XfbinParser.cfg")) {
        std::cout << "> Config file not found. Creating new `XfbinParser.cfg` with defaults.\n";
        std::ofstream config_file("XfbinParser.cfg");
        config_file << "show_levels = true\nshow_warns = true\nshow_verbose = false\nshow_debug = false";
        config_file.close();
    }

    // Create TOML table from config file.
    toml::table config_table = toml::parse_file("XfbinParser.cfg");
    log_config.show_levels = config_table["show_levels"].as_boolean();
    log_config.show_warns = config_table["show_warns"].as_boolean();
    log_config.show_debug = config_table["show_debug"].as_boolean();
    log_config.show_verbose = config_table["show_verbose"].as_boolean();
}

void Log_Output(log_level level, const std::string& message, const std::string& source) {
    // Associate levels with words.
    static const std::string level_strings[7] = { "CRITICAL", "ERROR", "WARNING", "DEBUG", "VERBOSE", "SUCCESS", "INFO" };

    // Create log file if not already open.
    if (!log_file.is_open()) {
        // Load configurations.
        Load_Log_Config();
        log_file.open("XfbinParser.log", std::ios::out | std::ios::trunc);
    }

    // Create final log string.
    std::string final_log_str;
    // Check whether to create log based on level and configs.
    if ( CHECK_LOG_LEVEL_CONFIG(LOG_LEVEL_WARN, log_config.show_warns) && CHECK_LOG_LEVEL_CONFIG(LOG_LEVEL_DEBUG, log_config.show_debug) && CHECK_LOG_LEVEL_CONFIG(LOG_LEVEL_VERBOSE, log_config.show_verbose) ) {
        // Start log with ">"
        final_log_str = "> ";
        // If not from main project (e.g. a DLL), append project title.
        if (source != PROJECT_NAME) {
            final_log_str += "[" + source + "] ";
        }
        // Append log level string.
        if ( log_config.show_levels == true ) {
            final_log_str += "[" + level_strings[level] + "]: ";
        }
        // Append log message.
        final_log_str += message + "\n";
    }

    // Output log to file.
    log_file << final_log_str;
    log_file.flush();

    // Output log to console.
    std::cout << final_log_str;
    if (level == LOG_LEVEL_CRITICAL) {
        End_Program();
    }
}