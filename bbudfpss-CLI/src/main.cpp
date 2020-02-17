
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
#include "archiver.h"

// Usage string text.
#define SZ_HELP_TEXT R"(bbudfpss
A simple back up tool for Windows.
Michael Skec, February 2020

    Command line arguments:
    -h, --help, /?
        Show this help dialogue.

    -i, --input, /I
        Specify input directory to back up.

    -o, --output, /O
        Specify directory to place the output archive.
)"

#define CMD_OK 1
#define CMD_HELP 2
#define CMD_FAIL -1

// Function prototypes.
void get_dir_files(std::vector<std::string>&, const std::string&, bool);
int parse_args(int, char**);

// Global variables
static char inputDir[4096];
static char outputDir[4096];

int main(int argc, char* argv[])
{
	// Parse arguments.
	int cmd = parse_args(argc, argv);
	if (cmd != CMD_OK)
	{
		// Invalid parameters were passed.
		if (cmd == CMD_FAIL)
		{
			std::cerr << "Aborting with parameter errors..." << std::endl;
			std::cin.get();
			return -1;
		}
		// Expected early exit.
		std::cin.get();
		return 0;
	}
	std::cout << "bbudfpss v0.1" << std::endl;

	// Make sure input and outputs is a directory.
	auto test_dir = [](const char* dir)
	{
		DWORD result = GetFileAttributesA(dir);
		if ((result & FILE_ATTRIBUTE_DIRECTORY) == 0)
		{
			std::cerr << "'" << dir << "' is not a directory!" << std::endl;
			std::cin.get();
			return 0;
		}
		if (result == INVALID_FILE_ATTRIBUTES)
		{
			std::cerr << "'" << dir << "' is an invalid directory. Make sure it exists." << std::endl;
			std::cin.get();
			return 0;
		}
		return 1;
	};
	if (!test_dir(inputDir))  
	{ 
		std::cin.get();
		return -1; 
	}
	if (!test_dir(outputDir)) 
	{ 
		std::cin.get();
		return -1; 
	}

	// Remove trailing slash from output dir.
	size_t outputSize = strlen(outputDir);
	char final_char = outputDir[outputSize - 1];
	if (final_char == '\\' || final_char == '/')
	{
		// Just put a null terminator char to shorten at slash.
		outputDir[outputSize - 1] = 0;
	}

	std::cout << std::endl << "Backing up '" << inputDir << "'..." << std::endl;
	std::cout << "Destination: '" << outputDir << "'" << std::endl;

	// Get all the files in the input directory,
	std::vector<std::string> v;
	get_dir_files(v, inputDir, true);
	
	// Get a timestamp string in format:
	// DDMMYY_HH-MM-SS
	char datestamp[6];
	char timestamp[6];
	SYSTEMTIME systime;
	GetLocalTime(&systime);
	GetDateFormatA(LOCALE_USER_DEFAULT, NULL, &systime, "ddMMyy", datestamp, 6);
	GetTimeFormatA(LOCALE_USER_DEFAULT, NULL, &systime, "'_'HH'-'mm", timestamp, 7);

	// Write to archive.
	char outname[MAX_PATH];
	strcpy_s(outname, outputDir);
	strcat_s(outname, "\\Backup_");
	strcat_s(outname, datestamp);
	strcat_s(outname, timestamp);
	strcat_s(outname, ".tar");
	write_archive(outname, v, inputDir);

	//
	// Compress the archive.
	//

	std::cout << std::endl << "Compressing archive..." << std::endl;

	// Create input stream from the archive
	std::ifstream infile(outname, std::ios_base::binary);
	infile.seekg(0, std::ios_base::end);
	size_t len = infile.tellg();
	infile.seekg(0, std::ios_base::beg);

	// Read bytes
	std::vector<char> buffer;
	buffer.reserve(len);
	std::copy(std::istreambuf_iterator<char>(infile),
		std::istreambuf_iterator<char>(),
		std::back_inserter(buffer));
	infile.close();

	// Replace uncompressed archive with the compressed one.
	std::remove(outname);
	strcat_s(outname, ".gz");

	const char* buff_uncomp = buffer.data();
	size_t buff_size_uncomp = buffer.size() * sizeof(char);

	zstr::ofstream outfile(outname);
	outfile.write(buff_uncomp, buff_size_uncomp);

	std::cout << "Done." << std::endl;
	return 0;
}

// Get a vector of all files in a directory.
// Will not work with Unicode currently!
// https://stackoverflow.com/questions/306533/how-do-i-get-a-list-of-files-in-a-directory-in-c
void get_dir_files(std::vector<std::string>& out, const std::string& directory, bool ignore_hidden)
{
	HANDLE dir;
	WIN32_FIND_DATAA file_data;

	if ((dir = FindFirstFileA((directory + "\\*").c_str(), &file_data))
		== INVALID_HANDLE_VALUE)
	{
		// No files found.
		return;
	}

	do
	{
		const std::string filename = file_data.cFileName;
		const std::string full_filename = directory + "\\" + filename;
		const bool is_dir = (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

		if (ignore_hidden && filename[0] == '.')
		{
			continue;
		}

		if (is_dir)
		{
			// Run it recursively into each directory too.
			// First we check that the directory isn't '.' or '..'
			size_t filename_len = filename.length();
			if (filename[0] == '.' && (filename_len == 1
				|| (filename_len == 2 && filename[1] == '.')))
			{
				continue;
			}

			get_dir_files(out, full_filename, ignore_hidden);
			continue;
		}

		// Push to vector.
		out.push_back(full_filename);
	} while (FindNextFileA(dir, &file_data));

	FindClose(dir);
}

// Parse command line arguments.
int parse_args(int argc, char* argv[])
{
	// Not enough args.
	if (argc == 1)
	{
		std::cout << "Missing parameters. Pass --help for usage information." << std::endl;
		return CMD_HELP;
	}

	// Iterate over arguments
	while (*(++argv))
	{
		// Help menu
		if (strcmp(*argv, "-h") == 0
			|| strcmp(*argv, "/?") == 0
			|| strcmp(*argv, "--help") == 0)
		{
			std::cout << SZ_HELP_TEXT << std::endl;
			return CMD_HELP;
		}

		// Input file.
		if (strcmp(*argv, "-i") == 0
			|| strcmp(*argv, "/I") == 0
			|| strcmp(*argv, "--input") == 0)
		{
			// Make sure next parameter exists
			if (!*(++argv))
			{
				std::cerr << "Missing input directory parameter." << std::endl;
				return CMD_FAIL;
			}

			// Get absolute path
			if (!GetFullPathNameA(*argv, 4096, inputDir, NULL))
			{
				std::cerr << "Invalid input file path" << std::endl;
				return CMD_FAIL;
			}

			continue;
		}

		// Input file.
		if (strcmp(*argv, "-o") == 0
			|| strcmp(*argv, "/O") == 0
			|| strcmp(*argv, "--output") == 0)
		{
			// Make sure next parameter exists.
			if (!*(++argv))
			{
				std::cerr << "Missing output directory parameter." << std::endl;
				return CMD_FAIL;
			}

			// Get absolute path
			if (!GetFullPathNameA(*argv, 4096, outputDir, NULL))
			{
				std::cerr << "Invalid output file path" << std::endl;
				return CMD_FAIL;
			}

			continue;
		}
	}

	// Make sure paths were passed.
	if (!inputDir)
	{
		std::cerr << "No input directory specified." << std::endl;
	}
	if (!outputDir)
	{
		std::cerr << "No output directory specified." << std::endl;
	}

	return CMD_OK;
}
