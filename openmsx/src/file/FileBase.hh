// $Id: FileBase.hh 11747 2010-11-11 07:15:09Z m9710797 $

#ifndef FILEBASE_HH
#define FILEBASE_HH

#include "MemBuffer.hh"
#include "openmsx.hh"
#include "noncopyable.hh"
#include <string>

namespace openmsx {

class FileBase : private noncopyable
{
public:
	FileBase();
	virtual ~FileBase();

	virtual void read(void* buffer, unsigned num) = 0;
	virtual void write(const void* buffer, unsigned num) = 0;

	// If you override mmap(), make sure to call munmap() in
	// your destructor.
	virtual const byte* mmap(unsigned& size);
	virtual void munmap();

	virtual unsigned getSize() = 0;
	virtual void seek(unsigned pos) = 0;
	virtual unsigned getPos() = 0;
	virtual void truncate(unsigned size);
	virtual void flush() = 0;

	virtual const std::string getURL() const = 0;
	virtual const std::string getLocalReference();
	virtual const std::string getOriginalName();
	virtual bool isReadOnly() const = 0;
	virtual time_t getModificationDate() = 0;

private:
	MemBuffer<byte> mmapBuf;
};

} // namespace openmsx

#endif
