#include "logger.h"
#include "files/files.h"

template<typename T, typename ... Cs>
constexpr bool IsAnyOf(const T& cmp, const Cs& ... others) {
	return ((cmp == others) || ...);
}

XFBIN xfbin; // Used across other files.

int main(int argc, char* argv[]) {
	std::cout << "~<|  XFBIN Parser++  |>~\n\n"; // header

	// If file not dragged onto EXE.
	if (argc < 2) {
		LOG_CRITICAL("No input file detected. Drag it onto this tool's EXE to get started!");
	}

	// Store input path to variable.
	fs::path input_path = argv[1];

	// Create main XFBIN object and load its data.
	if (input_path.extension() == ".xfbin") {
		LOG_INFO("XFBIN file detected. Loading data...");
		LOG_VERBOSE("Attempting to load `" + input_path.filename().string() + "` from path `" + input_path.string() + "`.");
		xfbin.Load_XFBIN(input_path);
		xfbin.Unpack();
	} else if (input_path.extension() == "") {
		LOG_INFO("Directory detected. Loading data...");
		LOG_VERBOSE("Attempting to load `_xfbin.json` from directory `" + input_path.string() + "`.");
		xfbin.Load_JSON(input_path / "_xfbin.json");
		xfbin.Repack();
	} else {
		LOG_CRITICAL("Invalid input type. Must be XFBIN or an unpacked directory.");
	}

	End_Program();
}