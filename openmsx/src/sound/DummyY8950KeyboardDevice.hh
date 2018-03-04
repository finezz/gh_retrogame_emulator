// $Id: DummyY8950KeyboardDevice.hh 12631 2012-06-14 20:18:24Z m9710797 $

#ifndef DUMMYY8950KEYBOARDDEVICE_HH
#define DUMMYY8950KEYBOARDDEVICE_HH

#include "Y8950KeyboardDevice.hh"

namespace openmsx {

class DummyY8950KeyboardDevice : public Y8950KeyboardDevice
{
public:
	virtual void write(byte data, EmuTime::param time);
	virtual byte read(EmuTime::param time);

	virtual string_ref getDescription() const;
	virtual void plugHelper(Connector& connector, EmuTime::param time);
	virtual void unplugHelper(EmuTime::param time);
};

} // namespace openmsx

#endif
