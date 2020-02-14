
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

// Function prototypes.
void get_dir_files(std::vector<std::string>&, const std::string&, bool);

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

	// Get absolute paths
	char inputDir[4096], outputDir[4096];
	if (!GetFullPathNameA(argv[1], 4096, inputDir, NULL))
	{
		std::cerr << "Invalid input file path" << std::endl;
	}
	if (!GetFullPathNameA(argv[2], 4096, outputDir, NULL))
	{
		std::cerr << "Invalid output file path" << std::endl;
	}

	// Make sure input is a directory.
	DWORD result = GetFileAttributesA(inputDir);
	if ((result & FILE_ATTRIBUTE_DIRECTORY) == 0)
	{
		std::cerr << "Input must be a directory." << std::endl;
		return -1;
	}
	if (result == INVALID_FILE_ATTRIBUTES)
	{
		std::cerr << "Invalid directory. Make sure it exists." << std::endl;
		return -1;
	}

	// Get all the files in the input directory,
	std::vector<std::string> v;
	get_dir_files(v, inputDir, true);

	// Create and put to a tarball.
	std::fstream out(std::string(outputDir).append("\\OUTPUT_TARBALL.tar").c_str(), std::ios::out);
	if (!out.is_open())
	{
		std::cerr << "Cannot open output file. Aborting..." << std::endl;
		std::cin.get();
		return -1;
	}
	tar::tarball archive(out);
	/*unsigned input_len = strlen(inputDir);
	for (unsigned i = 0; i < v.size(); ++i)
	{
		std::string_view relative_name(v[i].c_str() + input_len + 1, v[i].length() - input_len - 1);
		archive.putFile(v[i].c_str(), relative_name.data());
		//std::cout << v[i].c_str() << " as " << relative_name.data() << std::endl << std::endl;
	}*/
	archive.put("myfiles/item1.txt", "Hello World 1\n");
	archive.put("item2.txt", "Hello WorLd 1\n");
	archive.put("item3.txt", "Hello WORld 1\n");
	archive.put("item4.txt", "Hello world 1\n");
	archive.finish();
	out.close();

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
			unsigned filename_len = filename.length();
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
