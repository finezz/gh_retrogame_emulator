// $Id: RomAscii16_2.cc 12527 2012-05-17 17:34:11Z m9710797 $

// ASCII 16kB based cartridges with SRAM
//
// Examples: A-Train, Daisenryaku, Harry Fox MSX Special, Hydlide 2, Jyansei
//
// this type is is almost completely a ASCII16 cartrdige
// However, it has 2kB of SRAM (and 128 kB ROM)
// Use value 0x10 to select the SRAM.
// SRAM in page 1 => read-only
// SRAM in page 2 => read-write
// The 2Kb SRAM (0x800 bytes) are mirrored in the 16 kB block
//
// The address to change banks (from ASCII16):
//  first  16kb: 0x6000 - 0x67FF (0x6000 used)
//  second 16kb: 0x7000 - 0x77FF (0x7000 and 0x77FF used)

#include "RomAscii16_2.hh"
#include "Rom.hh"
#include "SRAM.hh"
#include "serialize.hh"

namespace openmsx {

RomAscii16_2::RomAscii16_2(const DeviceConfig& config, std::auto_ptr<Rom> rom)
	: RomAscii16kB(config, rom)
{
	sram.reset(new SRAM(getName() + " SRAM", 0x0800, config));
	reset(EmuTime::dummy());
}

RomAscii16_2::~RomAscii16_2()
{
}

void RomAscii16_2::reset(EmuTime::param dummy)
{
	sramEnabled = 0;
	RomAscii16kB::reset(dummy);
}

byte RomAscii16_2::readMem(word address, EmuTime::param time)
{
	if ((1 << (address >> 14)) & sramEnabled) {
		return (*sram)[address & 0x07FF];
	} else {
		return RomAscii16kB::readMem(address, time);
	}
}

const byte* RomAscii16_2::getReadCacheLine(word address) const
{
	if ((1 << (address >> 14)) & sramEnabled) {
		return &(*sram)[address & 0x07FF];
	} else {
		return RomAscii16kB::getReadCacheLine(address);
	}
}

void RomAscii16_2::writeMem(word address, byte value, EmuTime::param /*time*/)
{
	if ((0x6000 <= address) && (address < 0x7800) && !(address & 0x0800)) {
		// bank switch
		byte region = ((address >> 12) & 1) + 1;
		if (value == 0x10) {
			// SRAM block
			sramEnabled |= (1 << region);
			invalidateMemCache(0x4000 * region, 0x4000);
		} else {
			// ROM block
			setRom(region, value);
			sramEnabled &= ~(1 << region);
		}
	} else {
		// write sram
		if ((1 << (address >> 14)) & sramEnabled & 0x04) {
			sram->write(address & 0x07FF, value);
		}
	}
}

byte* RomAscii16_2::getWriteCacheLine(word address) const
{
	if ((1 << (address >> 14)) & sramEnabled & 0x04) {
		// write sram
		return NULL;
	} else {
		return RomAscii16kB::getWriteCacheLine(address);
	}
}

template<typename Archive>
void RomAscii16_2::serialize(Archive& ar, unsigned /*version*/)
{
	ar.template serializeBase<RomAscii16kB>(*this);
	ar.serialize("sramEnabled", sramEnabled);
}
INSTANTIATE_SERIALIZE_METHODS(RomAscii16_2);
REGISTER_MSXDEVICE(RomAscii16_2, "RomAscii16_2");

} // namespace openmsx
