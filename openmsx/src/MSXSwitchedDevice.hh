// $Id: MSXSwitchedDevice.hh 8352 2008-11-12 18:39:08Z m9710797 $

#ifndef MSXSWITCHEDDEVICE_HH
#define MSXSWITCHEDDEVICE_HH

#include "EmuTime.hh"
#include "noncopyable.hh"
#include "openmsx.hh"

namespace openmsx {

class MSXMotherBoard;

class MSXSwitchedDevice : private noncopyable
{
public:
	virtual byte readSwitchedIO(word port, EmuTime::param time) = 0;
	virtual byte peekSwitchedIO(word port, EmuTime::param time) const = 0;
	virtual void writeSwitchedIO(word port, byte value, EmuTime::param time) = 0;

protected:
	MSXSwitchedDevice(MSXMotherBoard& motherBoard, byte id);
	virtual ~MSXSwitchedDevice();

private:
	MSXMotherBoard& motherBoard;
	const byte id;
};

} // namespace openmsx

#endif
