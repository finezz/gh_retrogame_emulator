// $Id: DSKDiskImage.cc 12587 2012-06-06 20:31:32Z m9710797 $

#include "DSKDiskImage.hh"
#include "File.hh"

namespace openmsx {

DSKDiskImage::DSKDiskImage(const Filename& fileName)
	: SectorBasedDisk(fileName)
	, file(new File(fileName, File::PRE_CACHE))
{
	setNbSectors(file->getSize() / SECTOR_SIZE);
}

DSKDiskImage::DSKDiskImage(const Filename& fileName, const shared_ptr<File>& file_)
	: SectorBasedDisk(fileName)
	, file(file_)
{
	setNbSectors(file->getSize() / SECTOR_SIZE);
}

DSKDiskImage::~DSKDiskImage()
{
}

void DSKDiskImage::readSectorImpl(unsigned sector, byte* buf)
{
	file->seek(sector * SECTOR_SIZE);
	file->read(buf, SECTOR_SIZE);
}

void DSKDiskImage::writeSectorImpl(unsigned sector, const byte* buf)
{
	file->seek(sector * SECTOR_SIZE);
	file->write(buf, SECTOR_SIZE);
}

bool DSKDiskImage::isWriteProtectedImpl() const
{
	return file->isReadOnly();
}

Sha1Sum DSKDiskImage::getSha1Sum()
{
	if (hasPatches()) {
		return SectorAccessibleDisk::getSha1Sum();
	}
	return file->getSha1Sum();
}

} // namespace openmsx
