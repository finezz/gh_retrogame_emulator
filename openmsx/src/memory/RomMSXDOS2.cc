// $Id: RomMSXDOS2.cc 12527 2012-05-17 17:34:11Z m9710797 $

#include "RomMSXDOS2.hh"
#include "Rom.hh"
#include "CacheLine.hh"
#include "MSXException.hh"
#include "serialize.hh"
#include "unreachable.hh"

namespace openmsx {

RomMSXDOS2::RomMSXDOS2(const DeviceConfig& config, std::auto_ptr<Rom> rom_)
	: Rom16kBBlocks(config, rom_)
	, range((*rom)[0x94])
{
	if ((range != 0x00) && (range != 0x60) && (range != 0x7f)) {
		throw MSXException("Invalid rom for MSXDOS2 mapper");
	}
	reset(EmuTime::dummy());
}

void RomMSXDOS2::reset(EmuTime::param /*time*/)
{
	setUnmapped(0);
	setRom(1, 0);
	setUnmapped(2);
	setUnmapped(3);
}

void RomMSXDOS2::writeMem(word address, byte value, EmuTime::param /*time*/)
{
	switch (range) {
	case 0x00:
		if (address != 0x7ff0) return;
		break;
	case 0x60:
		if ((address & 0xf000) != 0x6000) return;
		break;
	case 0x7f:
		if (address != 0x7ffe) return;
		break;
	default:
		UNREACHABLE;
	}
	setRom(1, value);
}

byte* RomMSXDOS2::getWriteCacheLine(word address) const
{
	switch (range) {
	case 0x00:
		if (address == (0x7ff0 & CacheLine::HIGH)) return NULL;
		break;
	case 0x60:
		if ((address & 0xf000) == 0x6000) return NULL;
		break;
	case 0x7f:
		if (address == (0x7ffe & CacheLine::HIGH)) return NULL;
		break;
	default:
		UNREACHABLE;
	}
	return unmappedWrite;
}

REGISTER_MSXDEVICE(RomMSXDOS2, "RomMSXDOS2");

} // namespace openmsx
