// $Id: RomAscii16kB.cc 12527 2012-05-17 17:34:11Z m9710797 $

// ASCII 16kB cartridges
//
// this type is used in a few cartridges.
// example of cartridges: Xevious, Fantasy Zone 2,
// Return of Ishitar, Androgynus, Gallforce ...
//
// The address to change banks:
//  first  16kb: 0x6000 - 0x67ff (0x6000 used)
//  second 16kb: 0x7000 - 0x77ff (0x7000 and 0x77ff used)

#include "RomAscii16kB.hh"
#include "Rom.hh"
#include "serialize.hh"

namespace openmsx {

RomAscii16kB::RomAscii16kB(const DeviceConfig& config, std::auto_ptr<Rom> rom)
	: Rom16kBBlocks(config, rom)
{
	reset(EmuTime::dummy());
}

void RomAscii16kB::reset(EmuTime::param /*time*/)
{
	setUnmapped(0);
	setRom(1, 0);
	setRom(2, 0);
	setUnmapped(3);
}

void RomAscii16kB::writeMem(word address, byte value, EmuTime::param /*time*/)
{
	if ((0x6000 <= address) && (address < 0x7800) && !(address & 0x0800)) {
		byte region = ((address >> 12) & 1) + 1;
		setRom(region, value);
	}
}

byte* RomAscii16kB::getWriteCacheLine(word address) const
{
	if ((0x6000 <= address) && (address < 0x7800) && !(address & 0x0800)) {
		return NULL;
	} else {
		return unmappedWrite;
	}
}

REGISTER_MSXDEVICE(RomAscii16kB, "RomAscii16kB");

} // namespace openmsx
