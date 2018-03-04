// $Id: MSXMatsushita.hh 12527 2012-05-17 17:34:11Z m9710797 $

#ifndef MSXMATSUSHITA_HH
#define MSXMATSUSHITA_HH

#include "MSXDevice.hh"
#include "MSXSwitchedDevice.hh"
#include <memory>

namespace openmsx {

class FirmwareSwitch;
class SRAM;

class MSXMatsushita : public MSXDevice, public MSXSwitchedDevice
{
public:
	explicit MSXMatsushita(const DeviceConfig& config);
	virtual ~MSXMatsushita();

	// MSXDevice
	virtual void reset(EmuTime::param time);

	// MSXSwitchedDevice
	virtual byte readSwitchedIO(word port, EmuTime::param time);
	virtual byte peekSwitchedIO(word port, EmuTime::param time) const;
	virtual void writeSwitchedIO(word port, byte value, EmuTime::param time);

	template<typename Archive>
	void serialize(Archive& ar, unsigned version);

private:
	const std::auto_ptr<FirmwareSwitch> firmwareSwitch;
	const std::auto_ptr<SRAM> sram;
	word address;
	nibble color1, color2;
	byte pattern;
};

} // namespace openmsx

#endif
