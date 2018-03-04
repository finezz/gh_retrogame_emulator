// $Id: MSXFDC.cc 12583 2012-06-05 17:51:52Z m9710797 $

#include "MSXFDC.hh"
#include "Rom.hh"
#include "RealDrive.hh"
#include "XMLElement.hh"
#include "StringOp.hh"
#include "MSXException.hh"
#include "serialize.hh"

namespace openmsx {

MSXFDC::MSXFDC(const DeviceConfig& config)
	: MSXDevice(config)
	, rom(new Rom(getName() + " ROM", "rom", config))
{
	bool singleSided = config.findChild("singlesided") != NULL;
	int numDrives = config.getChildDataAsInt("drives", 1);
	if ((0 > numDrives) || (numDrives >= 4)) {
		throw MSXException(StringOp::Builder() <<
			"Invalid number of drives: " << numDrives);
	}
	unsigned timeout = config.getChildDataAsInt("motor_off_timeout_ms", 0);
	const XMLElement* styleEl = config.findChild("connectionstyle");
	bool signalsNeedMotorOn = !styleEl || (styleEl->getData() == "Philips");
	EmuDuration motorTimeout = EmuDuration::msec(timeout);
	int i = 0;
	for ( ; i < numDrives; ++i) {
		drives[i].reset(new RealDrive(
			getMotherBoard(), motorTimeout, signalsNeedMotorOn,
			!singleSided));
	}
	for ( ; i < 4; ++i) {
		drives[i].reset(new DummyDrive());
	}
}

MSXFDC::~MSXFDC()
{
}

void MSXFDC::powerDown(EmuTime::param time)
{
	for (int i = 0; i < 4; ++i) {
		drives[i]->setMotor(false, time);
	}
}

byte MSXFDC::readMem(word address, EmuTime::param /*time*/)
{
	return *MSXFDC::getReadCacheLine(address);
}

byte MSXFDC::peekMem(word address, EmuTime::param /*time*/) const
{
	return *MSXFDC::getReadCacheLine(address);
}

const byte* MSXFDC::getReadCacheLine(word start) const
{
	return &(*rom)[start & 0x3FFF];
}


template<typename Archive>
void MSXFDC::serialize(Archive& ar, unsigned /*version*/)
{
	ar.template serializeBase<MSXDevice>(*this);

	// Drives are already constructed at this point, so we cannot use the
	// polymorphic object construction of the serialization framework.
	// Destroying and reconstructing the drives is not an option because
	// DriveMultiplexer already has pointers to the drives.
	char tag[7] = { 'd', 'r', 'i', 'v', 'e', 'X', 0 };
	for (int i = 0; i < 4; ++i) {
		if (RealDrive* drive = dynamic_cast<RealDrive*>(drives[i].get())) {
			tag[5] = char('a' + i);
			ar.serialize(tag, *drive);
		}
	}
}
INSTANTIATE_SERIALIZE_METHODS(MSXFDC);

} // namespace openmsx
