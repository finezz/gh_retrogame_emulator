// $Id: DummyCassetteDevice.cc 12631 2012-06-14 20:18:24Z m9710797 $

#include "DummyCassetteDevice.hh"

namespace openmsx {

void DummyCassetteDevice::setMotor(bool /*status*/, EmuTime::param /*time*/)
{
	// do nothing
}

short DummyCassetteDevice::readSample(EmuTime::param /*time*/)
{
	return 32767; // TODO check value
}

void DummyCassetteDevice::setSignal(bool /*output*/, EmuTime::param /*time*/)
{
	// do nothing
}

string_ref DummyCassetteDevice::getDescription() const
{
	return "";
}

void DummyCassetteDevice::plugHelper(Connector& /*connector*/,
                                     EmuTime::param /*time*/)
{
}

void DummyCassetteDevice::unplugHelper(EmuTime::param /*time*/)
{
}

} // namespace openmsx
