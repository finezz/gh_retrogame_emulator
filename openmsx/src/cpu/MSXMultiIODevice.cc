// $Id: MSXMultiIODevice.cc 12528 2012-05-17 17:36:10Z m9710797 $

#include "MSXMultiIODevice.hh"
#include "StringOp.hh"
#include <algorithm>
#include <cassert>

namespace openmsx {

MSXMultiIODevice::MSXMultiIODevice(const HardwareConfig& hwConf)
	: MSXMultiDevice(hwConf)
{
}

MSXMultiIODevice::~MSXMultiIODevice()
{
	assert(devices.empty());
}

void MSXMultiIODevice::addDevice(MSXDevice* device)
{
	assert(std::count(devices.begin(), devices.end(), device) == 0);
	devices.push_back(device);
}

void MSXMultiIODevice::removeDevice(MSXDevice* device)
{
	assert(std::count(devices.begin(), devices.end(), device) == 1);
	devices.erase(std::find(devices.begin(), devices.end(), device));
}

MSXMultiIODevice::Devices& MSXMultiIODevice::getDevices()
{
	return devices;
}

std::string MSXMultiIODevice::getName() const
{
	assert(!devices.empty());
	StringOp::Builder result;
	result << devices[0]->getName();
	for (unsigned i = 1; i < devices.size(); ++i) {
		result << "  " << devices[i]->getName();
	}
	return result;
}

byte MSXMultiIODevice::readIO(word port, EmuTime::param time)
{
	// conflict: return the result from the first device, call readIO()
	//           also on all other devices, but discard result
	assert(!devices.empty());
	Devices::iterator it = devices.begin();
	byte result = (*it)->readIO(port, time);
	for (++it; it != devices.end(); ++it) {
		(*it)->readIO(port, time);
	}
	return result;
}

byte MSXMultiIODevice::peekIO(word port, EmuTime::param time) const
{
	// conflict: just peek first device
	assert(!devices.empty());
	return devices.front()->peekIO(port, time);
}

void MSXMultiIODevice::writeIO(word port, byte value, EmuTime::param time)
{
	for (Devices::iterator it = devices.begin();
	     it != devices.end(); ++it) {
		(*it)->writeIO(port, value, time);
	}
}

} // namespace openmsx
