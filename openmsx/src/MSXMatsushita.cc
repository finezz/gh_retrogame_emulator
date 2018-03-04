// $Id: MSXMatsushita.cc 12528 2012-05-17 17:36:10Z m9710797 $

#include "MSXMatsushita.hh"
#include "MSXCPU.hh"
#include "FirmwareSwitch.hh"
#include "SRAM.hh"
#include "serialize.hh"

namespace openmsx {

static const byte ID = 0x08;

MSXMatsushita::MSXMatsushita(const DeviceConfig& config)
	: MSXDevice(config)
	, MSXSwitchedDevice(getMotherBoard(), ID)
	, firmwareSwitch(new FirmwareSwitch(config))
	, sram(new SRAM(getName() + " SRAM", 0x800, config))
{
	// TODO find out what ports 0x41 0x45 0x46 are used for

	reset(EmuTime::dummy());
}

MSXMatsushita::~MSXMatsushita()
{
}

void MSXMatsushita::reset(EmuTime::param /*time*/)
{
	color1 = color2 = pattern = address = 0; // TODO check this
}

byte MSXMatsushita::readSwitchedIO(word port, EmuTime::param time)
{
	// TODO: Port 7 and 8 can be read as well.
	byte result = peekSwitchedIO(port, time);
	switch (port & 0x0F) {
	case 3:
		pattern = (pattern << 2) | (pattern >> 6);
		break;
	case 9:
		address = (address + 1) & 0x1FFF;
		break;
	}
	return result;
}

byte MSXMatsushita::peekSwitchedIO(word port, EmuTime::param /*time*/) const
{
	byte result;
	switch (port & 0x0F) {
	case 0:
		result = byte(~ID);
		break;
	case 1:
		result = firmwareSwitch->getStatus() ? 0x7F : 0xFF;
		break;
	case 3:
		result = (((pattern & 0x80) ? color2 : color1) << 4)
		        | ((pattern & 0x40) ? color2 : color1);
		break;
	case 9:
		if (address < 0x800) {
			result = (*sram)[address];
		} else {
			result = 0xFF;
		}
		break;
	default:
		result = 0xFF;
	}
	return result;
}

void MSXMatsushita::writeSwitchedIO(word port, byte value, EmuTime::param /*time*/)
{
	switch (port & 0x0F) {
	case 1:
		if (value & 1) {
			// bit0 = 1 -> 3.5MHz
			getCPU().setZ80Freq(3579545);
		} else {
			// bit0 = 0 -> 5.3MHz
			getCPU().setZ80Freq(5369318); // 3579545 * 3/2
		}
		break;
	case 3:
		color2 = (value & 0xF0) >> 4;
		color1 =  value & 0x0F;
		break;
	case 4:
		pattern = value;
		break;
	case 7:
		// set address (low)
		address = (address & 0xFF00) | value;
		break;
	case 8:
		// set address (high)
		address = (address & 0x00FF) | ((value & 0x1F) << 8);
		break;
	case 9:
		// write sram
		if (address < 0x800) {
			sram->write(address, value);
		}
		address = (address + 1) & 0x1FFF;
		break;
	}
}

template<typename Archive>
void MSXMatsushita::serialize(Archive& ar, unsigned /*version*/)
{
	ar.template serializeBase<MSXDevice>(*this);
	// no need to serialize MSXSwitchedDevice base class

	ar.serialize("SRAM", *sram);
	ar.serialize("address", address);
	ar.serialize("color1", color1);
	ar.serialize("color2", color2);
	ar.serialize("pattern", pattern);
}
INSTANTIATE_SERIALIZE_METHODS(MSXMatsushita);
REGISTER_MSXDEVICE(MSXMatsushita, "Matsushita");

} // namespace openmsx
