/*
 * tarball.cpp
 *
 *  Created on: Jul 28, 2010
 *      Author: Pierre Lindenbaum PhD
 *              plindenbaum@yahoo.fr
 *              http://plindenbaum.blogspot.com
 *
 */
#include <iostream>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <ctime>
#ifndef PLATFORM_WINDOWS
#   include <unistd.h>
#   define GET_USERNAME ::getlogin()
#else
#   include <io.h>
#   include <Windows.h>
#   include <Lmcons.h>
#   define GET_USERNAME "bbudfpss" // Win_get_username()

    // Get username. Only ASCII supported...
    std::string Win_get_username()
	{
        char username[UNLEN + 1];
        DWORD username_len = UNLEN + 1;
        GetUserNameA(username, &username_len);
        return std::string(username);
	}
#endif
#include "tarball.h"
#include "throw.h"
#define LOCALNS tar
#define TARHEADER static_cast<PosixTarHeader*>(header)

struct PosixTarHeader
{
	char name[100];
	char mode[8];
	char uid[8];
	char gid[8];
	char size[12];
	char mtime[12];
	char checksum[8];
	char typeflag[1];
	char linkname[100];
	char magic[6];
	char version[2];
	char uname[32];
	char gname[32];
	char devmajor[8];
	char devminor[8];
	char prefix[155];
	char pad[12];
};



void LOCALNS::tarball::_init(void* header)
{
    std::memset(header, 0, sizeof(PosixTarHeader));
    strcpy_s(TARHEADER->magic, "ustar");
    strcpy_s(TARHEADER->version, " ");
    sprintf_s(TARHEADER->mtime, "%011lo", (unsigned long)time(NULL));
    sprintf_s(TARHEADER->mode, "%07o", 0644);
    char* s = (char*)GET_USERNAME;
    if (s != NULL)
    {
		std::snprintf((char*)header, 32, "%s", s);
    }
    sprintf_s(TARHEADER->gname, "%s", "users");
}

void LOCALNS::tarball::_checksum(void* header)
{
    unsigned int sum = 0;
    char *p = (char *) header;
    char *q = p + sizeof(PosixTarHeader);
    while (p < TARHEADER->checksum) sum += *p++ & 0xff;
    for (int i = 0; i < 8; ++i)  
    {
	  sum += ' ';
	  ++p;
	}
    while (p < q) sum += *p++ & 0xff;

    sprintf_s(TARHEADER->checksum,"%06o",sum);
}

void LOCALNS::tarball::_size(void* header,unsigned long fileSize)
{
    sprintf_s(TARHEADER->size,"%011llo",(long long unsigned int)fileSize);
}

void LOCALNS::tarball::_filename(void* header,const char* filename)
{
    if(filename==NULL || filename[0]==0 || std::strlen(filename)>=100)
	{
		THROW("invalid archive name \"" << filename << "\"");
	}
    std::snprintf(TARHEADER->name,100,"%s",filename);
}

void LOCALNS::tarball::_endRecord(std::size_t len)
{
    char c='\0';
    while((len%sizeof(PosixTarHeader))!=0)
	{
		out.write(&c,sizeof(char));
		++len;
	}
}


LOCALNS::tarball::tarball(std::ostream& out):_finished(false),out(out)
{
    if(sizeof(PosixTarHeader)!=512)
	{
		THROW(sizeof(PosixTarHeader));
	}
}

LOCALNS::tarball::~tarball()
{
    if(!_finished)
	{
		std::cerr << "[warning]tar file was not finished."<< std::endl;
	}
}

/** writes 2 empty blocks. Should be always called before closing the Tar file */
void LOCALNS::tarball::finish()
{
    _finished=true;
    // The end of the archive is indicated by two blocks filled with binary zeros
    PosixTarHeader header;
    std::memset((void*)&header,0,sizeof(PosixTarHeader));
    out.write((const char*)&header,sizeof(PosixTarHeader));
    out.write((const char*)&header,sizeof(PosixTarHeader));
    out.flush();
}

void LOCALNS::tarball::put(const char* filename,const std::string& s)
{
	put(filename,s.c_str(),s.size());
}
void LOCALNS::tarball::put(const char* filename,const char* content)
{
	put(filename,content,std::strlen(content));
}

void LOCALNS::tarball::put(const char* filename,const char* content,std::size_t len)
{
    PosixTarHeader header;
    _init((void*)&header);
    _filename((void*)&header,filename);
    header.typeflag[0]=0;
    _size((void*)&header,len);
    _checksum((void*)&header);
    out.write((const char*)&header,sizeof(PosixTarHeader));
    out.write(content,len);
    _endRecord(len);
}

void LOCALNS::tarball::putFile(const char* filename,const char* nameInArchive)
{
    char buff[BUFSIZ];
    FILE* in;
    if (!fopen_s(&in, filename, "rb") == 0)
	{
		THROW("Cannot open " << filename);
	}
    std::fseek(in, 0L, SEEK_END);
    long int len= std::ftell(in);
    std::fseek(in,0L,SEEK_SET);

    PosixTarHeader header;
    _init((void*)&header);
    _filename((void*)&header,nameInArchive);
    header.typeflag[0]=0;
    _size((void*)&header,len);
    _checksum((void*)&header);
    out.write((const char*)&header,sizeof(PosixTarHeader));


    std::size_t nRead=0;
    while((nRead=std::fread(buff,sizeof(char),BUFSIZ,in))>0)
	{
		out.write(buff,nRead);
	}
    std::fclose(in);

    _endRecord(len);
}
