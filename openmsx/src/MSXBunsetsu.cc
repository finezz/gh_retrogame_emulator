// $Id: MSXBunsetsu.cc 12527 2012-05-17 17:34:11Z m9710797 $

#include "MSXBunsetsu.hh"
#include "CacheLine.hh"
#include "Rom.hh"
#include "serialize.hh"

namespace openmsx {

MSXBunsetsu::MSXBunsetsu(const DeviceConfig& config)
	: MSXDevice(config)
	, bunsetsuRom(new Rom(getName() + "_1", "rom", config, "bunsetsu"))
	, jisyoRom   (new Rom(getName() + "_2", "rom", config, "jisyo"))
{
	reset(EmuTime::dummy());
}

MSXBunsetsu::~MSXBunsetsu()
{
}

void MSXBunsetsu::reset(EmuTime::param /*time*/)
{
	jisyoAddress = 0;
}

byte MSXBunsetsu::readMem(word address, EmuTime::param /*time*/)
{
	byte result;
	if (address == 0xBFFF) {
		result = (*jisyoRom)[jisyoAddress];
		jisyoAddress = (jisyoAddress + 1) & 0x1FFFF;
	} else if ((0x4000 <= address) && (address < 0xC000)) {
		result = (*bunsetsuRom)[address - 0x4000];
	} else {
		result = 0xFF;
	}
	return result;
}

void MSXBunsetsu::writeMem(word address, byte value, EmuTime::param /*time*/)
{
	switch (address) {
	case 0xBFFC:
		jisyoAddress = (jisyoAddress & 0x1FF00) | value;
		break;
	case 0xBFFD:
		jisyoAddress = (jisyoAddress & 0x100FF) | (value << 8);
		break;
	case 0xBFFE:
		jisyoAddress = (jisyoAddress & 0x0FFFF) |
		               ((value & 1) << 16);
		break;
	}
}

const byte* MSXBunsetsu::getReadCacheLine(word start) const
{
	if ((start & CacheLine::HIGH) == (0xBFFF & CacheLine::HIGH)) {
		return NULL;
	} else {
		return &(*bunsetsuRom)[start - 0x4000];
	}
}

byte* MSXBunsetsu::getWriteCacheLine(word start) const
{
	if ((start & CacheLine::HIGH) == (0xBFFF & CacheLine::HIGH)) {
		return NULL;
	} else {
		return unmappedWrite;
	}
}

template<typename Archive>
void MSXBunsetsu::serialize(Archive& ar, unsigned /*version*/)
{
	ar.template serializeBase<MSXDevice>(*this);
	ar.serialize("jisyoAddress", jisyoAddress);
}
INSTANTIATE_SERIALIZE_METHODS(MSXBunsetsu);
REGISTER_MSXDEVICE(MSXBunsetsu, "Bunsetsu");

} // namespace openmsx
