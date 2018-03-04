// $Id: ZipFileAdapter.cc 11747 2010-11-11 07:15:09Z m9710797 $

#include "ZipFileAdapter.hh"
#include "ZlibInflate.hh"
#include "FileException.hh"

namespace openmsx {

ZipFileAdapter::ZipFileAdapter(std::auto_ptr<FileBase> file_)
	: CompressedFileAdapter(file_)
{
}

void ZipFileAdapter::decompress(FileBase& file, Decompressed& decompressed)
{
	unsigned size;
	const byte* data = file.mmap(size);
	ZlibInflate zlib(data, size);

	if (zlib.get32LE() != 0x04034B50) {
		throw FileException("Invalid ZIP file");
	}

	// skip "version needed to extract" and "general purpose bit flag"
	zlib.skip(2 + 2);

	// compression method
	if (zlib.get16LE() != 0x0008) {
		throw FileException("Unsupported zip compression method");
	}

	// skip "last mod file time", "last mod file data",
	//      "crc32",              "compressed size"
	zlib.skip(2 + 2 + 4 + 4);

	unsigned origSize = zlib.get32LE(); // uncompressed size
	unsigned filenameLen = zlib.get16LE(); // filename length
	unsigned extraFieldLen = zlib.get16LE(); // extra field length
	decompressed.originalName = zlib.getString(filenameLen); // original filename
	zlib.skip(extraFieldLen); // skip "extra field"

	zlib.inflate(decompressed.buf, origSize);
}

} // namespace openmsx
