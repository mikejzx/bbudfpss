#include "pch.h"

/*
	Writes an archive with all files in filenames vector.
*/
void write_archive(const char* outname, const std::vector<std::string>& filenames, const char* indir)
{
	archive* a;
	archive_entry* entry;
	struct stat st;
	char buff[8192];
	size_t len;
	std::FILE* fd;

	a = archive_write_new();
	//archive_write_add_filter_gzip(a);
	archive_write_set_format_pax_restricted(a);
	archive_write_open_filename(a, outname);
	size_t fc = filenames.size();
	size_t inl = strlen(indir);
	unsigned i;
	for (i = 0; i < fc; ++i)
	{
		const char* fn = filenames[i].c_str();
		stat(fn, &st);
		entry = archive_entry_new();

		const char* rname = std::string_view(fn + inl + 1, strlen(fn) - inl - 1).data();

		archive_entry_set_pathname(entry, rname);
		archive_entry_set_size(entry, st.st_size);
		archive_entry_set_filetype(entry, AE_IFREG);
		archive_entry_set_perm(entry, 0644);
		archive_write_header(a, entry);

		if (fopen_s(&fd, fn, "rb") != 0)
		{
			std::cerr << "Couldn't open file " << fn << std::endl;
			archive_entry_free(entry);
			continue;
		}
		len = fread(buff, sizeof(char), sizeof buff, fd);
		while (len > 0)
		{
			archive_write_data(a, buff, len);
			len = fread(buff, sizeof(char), sizeof buff, fd);
		}
		fclose(fd);
		archive_entry_free(entry);
	}
	archive_write_close(a);
	archive_write_free(a);

	std::cout << "Wrote " << i << " files to archive." << std::endl;
}