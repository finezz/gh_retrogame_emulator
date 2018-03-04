// $Id: NowindRomDisk.cc 10532 2009-09-06 23:03:48Z mthuurne $

#include "NowindRomDisk.hh"
#include "serialize.hh"
#include "serialize_meta.hh"

namespace openmsx {

SectorAccessibleDisk* NowindRomDisk::getSectorAccessibleDisk()
{
	return NULL;
}

const std::string& NowindRomDisk::getContainerName() const
{
	static const std::string NAME = "NowindRomDisk";
	return NAME;
}

bool NowindRomDisk::diskChanged()
{
	return false;
}

int NowindRomDisk::insertDisk(const std::string& /*filename*/)
{
	return -1; // Can't change NowindRomDisk disk image
}

template<typename Archive>
void NowindRomDisk::serialize(Archive& /*ar*/, unsigned /*version*/)
{
}
INSTANTIATE_SERIALIZE_METHODS(NowindRomDisk);
REGISTER_POLYMORPHIC_CLASS(DiskContainer, NowindRomDisk, "NowindRomDisk");

} // namespace openmsx
