// $Id: LocalFile.hh 12621 2012-06-14 20:10:52Z m9710797 $

#ifndef LOCALFILE_HH
#define LOCALFILE_HH

#if defined _WIN32
#include <windows.h>
#endif
#include "File.hh"
#include "FileBase.hh"
#include "systemfuncs.hh"
#include <cstdio>
#include <memory>

namespace openmsx {

class PreCacheFile;

class LocalFile : public FileBase
{
public:
	LocalFile(string_ref filename, File::OpenMode mode);
	LocalFile(string_ref filename, const char* mode);
	virtual ~LocalFile();
	virtual void read (void* buffer, unsigned num);
	virtual void write(const void* buffer, unsigned num);
#if HAVE_MMAP || defined _WIN32
	virtual const byte* mmap(unsigned& size);
	virtual void munmap();
#endif
	virtual unsigned getSize();
	virtual void seek(unsigned pos);
	virtual unsigned getPos();
#if HAVE_FTRUNCATE
	virtual void truncate(unsigned size);
#endif
	virtual void flush();
	virtual const std::string getURL() const;
	virtual const std::string getLocalReference();
	virtual bool isReadOnly() const;
	virtual time_t getModificationDate();

	void preCacheFile();

private:
	std::string filename;
	FILE* file;
#if HAVE_MMAP
	byte* mmem;
#endif
#if defined _WIN32
	byte* mmem;
	HANDLE hMmap;
#endif
	std::auto_ptr<PreCacheFile> cache;
	bool readOnly;
};

} // namespace openmsx

#endif
