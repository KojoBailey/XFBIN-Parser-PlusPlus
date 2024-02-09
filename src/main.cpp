#include "logger.h"
#include "files/files.h"

template<typename T, typename ... Cs>
constexpr bool IsAnyOf(const T& cmp, const Cs& ... others) {
	return ((cmp == others) || ...);
}

void End(int exitCode) {
    std::cout << "Press enter to exit...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::exit(exitCode);
}

int main(int argc, char* argv[]) {
	// For logging useful information.
	Log log;
	log.file.open("Log.txt");

	// If file not dragged onto EXE.
	if (argc < 2) {
		log.Error("No file input.");
		End();
	}

	// Get input file.
	File input;
	if (input.LoadFile(argv[1]) == -1) {
		log.Error("\"" + input.name + input.extension + "\" could not be located or opened.\n");
		End();
	}
	if (!IsAnyOf(input.extension, ".json", ".xfbin", ".bin.xfbin")) {
		log.Error("File must be in the format of JSON or XFBIN.");
		End();
	}
	input.ReadFileToVector();
	log.Send("File \"" + input.name + input.extension + "\" successfully loaded. Preparing to parse...");

	input.Unpack();
}