// $Id: RomGeneric8kB.cc 12527 2012-05-17 17:34:11Z m9710797 $

#include "RomGeneric8kB.hh"
#include "Rom.hh"
#include "serialize.hh"

namespace openmsx {

RomGeneric8kB::RomGeneric8kB(const DeviceConfig& config, std::auto_ptr<Rom> rom)
	: Rom8kBBlocks(config, rom)
{
	reset(EmuTime::dummy());
}

void RomGeneric8kB::reset(EmuTime::param /*time*/)
{
	setUnmapped(0);
	setUnmapped(1);
	for (int i = 2; i < 6; i++) {
		setRom(i, i - 2);
	}
	setUnmapped(6);
	setUnmapped(7);
}

void RomGeneric8kB::writeMem(word address, byte value, EmuTime::param /*time*/)
{
	setRom(address >> 13, value);
}

byte* RomGeneric8kB::getWriteCacheLine(word address) const
{
	if ((0x4000 <= address) && (address < 0xC000)) {
		return NULL;
	} else {
		return unmappedWrite;
	}
}

REGISTER_MSXDEVICE(RomGeneric8kB, "RomGeneric8kB");

} // namespace openmsx
