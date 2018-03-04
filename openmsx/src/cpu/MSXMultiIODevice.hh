// $Id: MSXMultiIODevice.hh 12528 2012-05-17 17:36:10Z m9710797 $

#ifndef MSXMULTIIODEVICE_HH
#define MSXMULTIIODEVICE_HH

#include "MSXMultiDevice.hh"
#include <vector>

namespace openmsx {

class MSXMultiIODevice : public MSXMultiDevice
{
public:
	typedef std::vector<MSXDevice*> Devices;

	explicit MSXMultiIODevice(const HardwareConfig& hwConf);
	virtual ~MSXMultiIODevice();

	void addDevice(MSXDevice* device);
	void removeDevice(MSXDevice* device);
	Devices& getDevices();

	// MSXDevice
	virtual std::string getName() const;
	virtual byte readIO(word port, EmuTime::param time);
	virtual byte peekIO(word port, EmuTime::param time) const;
	virtual void writeIO(word port, byte value, EmuTime::param time);

private:
	Devices devices;
};

} // namespace openmsx

#endif
