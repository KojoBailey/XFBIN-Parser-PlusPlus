#pragma once
// See "logger.cpp" & "windows.cpp"

#include <string>

// If project name is redefined differently from as in 'logger.cpp', will show in the logs.
extern std::string THIS_PROJECT_NAME;

// Log levels. Lower level = higher priority.
typedef enum log_levels {
	LOG_LEVEL_CRITICAL = 0,
	LOG_LEVEL_ERROR = 1,
	LOG_LEVEL_WARN = 2,
	LOG_LEVEL_DEBUG = 3,
	LOG_LEVEL_VERBOSE = 4,
	LOG_LEVEL_SUCCESS = 5,
	LOG_LEVEL_INFO = 6
} log_level;

// Log configurations.
struct log_configs {
	bool show_levels;
	bool show_warns;
	bool show_debug;
	bool show_verbose;
};

// Load configuration data.
void Load_Log_Config();

// Check if showing of logs under certain level is enabled in configs (true if so).
#define CHECK_LOG_LEVEL_CONFIG(m_level, m_config) !(level == m_level && m_config == false)

// Function to pause and end program.
void End_Program(int exitCode = 0);

// Main function for outputting to log.
void Log_Output(log_level level, const std::string& message, const std::string& source);

// Macros for short logging.
#define LOG_CRITICAL(message) Log_Output(LOG_LEVEL_CRITICAL, message, THIS_PROJECT_NAME)
#define LOG_ERROR(message) Log_Output(LOG_LEVEL_ERROR, message, THIS_PROJECT_NAME)
#define LOG_WARN(message) Log_Output(LOG_LEVEL_WARN, message, THIS_PROJECT_NAME)
#define LOG_DEBUG(message) Log_Output(LOG_LEVEL_DEBUG, message, THIS_PROJECT_NAME)
#define LOG_VERBOSE(message) Log_Output(LOG_LEVEL_VERBOSE, message, THIS_PROJECT_NAME)
#define LOG_SUCCESS(message) Log_Output(LOG_LEVEL_SUCCESS, message, THIS_PROJECT_NAME)
#define LOG_INFO(message) Log_Output(LOG_LEVEL_INFO, message, THIS_PROJECT_NAME)