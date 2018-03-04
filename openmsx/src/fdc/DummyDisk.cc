// $Id: DummyDisk.cc 9383 2009-03-17 19:26:19Z m9710797 $

#include "DummyDisk.hh"
#include "DiskExceptions.hh"

namespace openmsx {

DummyDisk::DummyDisk()
	: SectorBasedDisk(Filename(""))
{
	setNbSectors(0);
}

bool DummyDisk::isDummyDisk() const
{
	return true;
}

bool DummyDisk::isWriteProtectedImpl() const
{
	return true; // TODO check
}

void DummyDisk::readSectorImpl(unsigned /*sector*/, byte* /*buf*/)
{
	throw DriveEmptyException("No disk in drive");
}

void DummyDisk::writeSectorImpl(unsigned /*sector*/, const byte* /*buf*/)
{
	throw DriveEmptyException("No disk in drive");
}

} // namespace openmsx
