// $Id: MSXS1985.hh 12742 2012-07-21 20:28:13Z manuelbi $

/*
 * This class implements the
 *   backup RAM
 *   bitmap function
 * of the S1985 MSX-engine
 *
 *  TODO explanation
 */

#ifndef S1985_HH
#define S1985_HH

#include "MSXDevice.hh"
#include "MSXSwitchedDevice.hh"
#include <memory>

namespace openmsx {

class SRAM;

class MSXS1985 : public MSXDevice, public MSXSwitchedDevice
{
public:
	explicit MSXS1985(const DeviceConfig& config);
	virtual ~MSXS1985();

	// MSXDevice
	virtual void reset(EmuTime::param time);

	// MSXSwitchedDevice
	virtual byte readSwitchedIO(word port, EmuTime::param time);
	virtual byte peekSwitchedIO(word port, EmuTime::param time) const;
	virtual void writeSwitchedIO(word port, byte value, EmuTime::param time);

	template<typename Archive>
	void serialize(Archive& ar, unsigned version);

private:
	std::auto_ptr<SRAM> sram;
	nibble address;
	byte color1;
	byte color2;
	byte pattern;
};
SERIALIZE_CLASS_VERSION(MSXS1985, 2);

} // namespace openmsx

#endif
