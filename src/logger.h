#pragma once

#include <iostream>
#include <fstream>

class Log {
public:
	std::ofstream file;

	void Send(std::string message) {
		std::string buffer = "> " + message + "\n";
		std::cout << buffer;
		file << buffer;
	}

	void Error(std::string message) {
		std::string buffer = "> ERROR: " + message + "\n";
		std::cerr << "\033[31m" << buffer << "\033[0m";
		file << buffer;
	}
};

void End(int exitCode = 0);