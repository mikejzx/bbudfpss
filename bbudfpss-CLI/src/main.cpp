
/*
	bbudfpss (Basic Backup Utility Designed for Proprietary Software-running Systems)
	- An extremely minimal tool for creating a compressed backup of a folder,
		and pushing it to an arbitrary folder. 
	- The resulting file will be given a proper timestamped name, and 
		compressed into a tarball or (in future) another archive format.
*/
/*
	main.cpp
	Entry point of the CLI application.
	The GUI application will be a front-end for this.
*/

// Include pre-compiled header.
#include "pch.h"

int main(int argc, char* argv[])
{
	std::cout << "bbudfpss v0.1" << std::endl;
	
	// Need at least two arguments.
	if (argc < 2)
	{
		std::cerr << "Missing parameters. Usage: bbudfpss <input directory> <output directory>" << std::endl;
		std::cin.get();
		return -1;
	}

	// Open file for write.
	std::fstream out("archive.tar", std::ios::out);
	if (!out.is_open())
	{
		std::cerr << "Cannot open out." << std::endl;
		std::cin.get();
		return -1;
	}

	// Create tarball.
	tar::tarball archive(out);
	archive.putFile("bbudfpss-CLI.vcxproj", "bbudfpss-CLI");
	archive.finish();
	out.close();

	return 0;
}