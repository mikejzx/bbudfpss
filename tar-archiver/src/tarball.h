/*
 * tarball.h
 *
 *  Created on: Jul 28, 2010
 *      Author: Pierre Lindenbaum PhD
 *              plindenbaum@yahoo.fr
 *              http://plindenbaum.blogspot.com
 *              
 */

#ifdef MAKEDLL
#	define TAR_API __declspec(dllexport)
#else
#	define TAR_API __declspec(dllimport)
#endif

#ifndef LINDENB_IO_TARBALL_H_
#define LINDENB_IO_TARBALL_H_

#include <iostream>

namespace tar 
{
	/**
	 *  A Tar Archive
	 */
	class TAR_API tarball
	{
	private:
		bool _finished;
	protected:
		std::ostream& out;
		void _init(void* header);
		void _checksum(void* header);
		void _size(void* header,unsigned long fileSize);
		void _filename(void* header,const char* filename);
		void _endRecord(std::size_t len);
	public:
		tarball(std::ostream& out);
		virtual ~tarball();
		/** writes 2 empty blocks. Should be always called before closing the tarball file */
		void finish();
		void put(const char* filename,const std::string& s);
		void put(const char* filename,const char* content);
		void put(const char* filename,const char* content,std::size_t len);
		void putFile(const char* filename,const char* nameInArchive);
	};
}

#endif /* LINDENB_IO_TARBALL_H_ */
