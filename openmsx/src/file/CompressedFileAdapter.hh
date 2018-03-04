// $Id: CompressedFileAdapter.hh 11747 2010-11-11 07:15:09Z m9710797 $

#ifndef COMPRESSEDFILEADAPTER_HH
#define COMPRESSEDFILEADAPTER_HH

#include "FileBase.hh"
#include "MemBuffer.hh"
#include "shared_ptr.hh"
#include <memory>

namespace openmsx {

class CompressedFileAdapter : public FileBase
{
public:
	struct Decompressed {
		MemBuffer<byte> buf;
		std::string originalName;
		std::string cachedURL;
		time_t cachedModificationDate;
	};

	virtual void read(void* buffer, unsigned num);
	virtual void write(const void* buffer, unsigned num);
	virtual const byte* mmap(unsigned& size);
	virtual void munmap();
	virtual unsigned getSize();
	virtual void seek(unsigned pos);
	virtual unsigned getPos();
	virtual void truncate(unsigned size);
	virtual void flush();
	virtual const std::string getURL() const;
	virtual const std::string getOriginalName();
	virtual bool isReadOnly() const;
	virtual time_t getModificationDate();

protected:
	explicit CompressedFileAdapter(std::auto_ptr<FileBase> file);
	virtual ~CompressedFileAdapter();
	virtual void decompress(FileBase& file, Decompressed& decompressed) = 0;

private:
	void decompress();

	std::auto_ptr<FileBase> file;
	shared_ptr<Decompressed> decompressed;
	unsigned pos;
};

} // namespace openmsx

#endif
