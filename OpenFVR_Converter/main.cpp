#include <iostream>

#include "Converter/converterpak.h"

void printHelp(char *programName)
{
	std::cout << "Usage: " << std::string(programName) << " <command> [parameters]\n";
	std::cout << "Command list:\n";
	std::cout << "\tunpack <pak_file> <output>\n";
	std::cout << "\t\tUnpacks 'pak_file' into 'output' (folder must exist)\n";
}

int main(int argc, char *argv[])
{
	if (argc > 1) {
		if (strcmp(argv[1], "unpack") == 0) {
			if (argc == 4) {
				ConverterPack::unpack(std::string(argv[2]), std::string(argv[3]));
			}
			else {
				std::cerr << "Invalid unpack parameters\n";
				std::cerr << "Run the program without parameters to print help\n";
			}
		}
		else if (strcmp(argv[1], "help") == 0) {
			printHelp(argv[0]);
		}
		else {
			std::cerr << "Unknown command: " << std::string(argv[1]) << "\n";
			std::cerr << "Run the program without parameters to print help\n";
		}
	}
	else {
		printHelp(argv[0]);
	}

	return 0;
}
