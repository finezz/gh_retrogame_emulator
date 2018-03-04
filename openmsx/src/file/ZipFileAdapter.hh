// $Id: ZipFileAdapter.hh 11571 2010-07-06 06:16:10Z m9710797 $

#ifndef ZIPFILEADAPTER_HH
#define ZIPFILEADAPTER_HH

#include "CompressedFileAdapter.hh"

namespace openmsx {

class ZipFileAdapter : public CompressedFileAdapter
{
public:
	explicit ZipFileAdapter(std::auto_ptr<FileBase> file);

private:
	virtual void decompress(FileBase& file, Decompressed& decompressed);
};

} // namespace openmsx

#endif
