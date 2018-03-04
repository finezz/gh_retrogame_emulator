// $Id: SETetrisDongle.hh 12631 2012-06-14 20:18:24Z m9710797 $

#ifndef SETETRISDONGLE_HH
#define SETETRISDONGLE_HH

#include "JoystickDevice.hh"

namespace openmsx {

class SETetrisDongle : public JoystickDevice
{
public:
	SETetrisDongle();

	// Pluggable
	virtual const std::string& getName() const;
	virtual string_ref getDescription() const;
	virtual void plugHelper(Connector& connector, EmuTime::param time);
	virtual void unplugHelper(EmuTime::param time);

	// JoystickDevice
	virtual byte read(EmuTime::param time);
	virtual void write(byte value, EmuTime::param time);

	template<typename Archive>
	void serialize(Archive& ar, unsigned version);

private:
	byte status;
};

} // namespace openmsx

#endif
