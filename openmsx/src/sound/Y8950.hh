// $Id: Y8950.hh 12527 2012-05-17 17:34:11Z m9710797 $

#ifndef Y8950_HH
#define Y8950_HH

#include "EmuTime.hh"
#include "openmsx.hh"
#include <string>
#include <memory>

namespace openmsx {

class MSXAudio;
class MSXMotherBoard;
class DeviceConfig;

class Y8950
{
public:
	static const int CLOCK_FREQ     = 3579545;
	static const int CLOCK_FREQ_DIV = 72;

	// Bitmask for register 0x04
	// Timer1 Start.
	static const int R04_ST1          = 0x01;
	// Timer2 Start.
	static const int R04_ST2          = 0x02;
	// not used
	//static const int R04            = 0x04;
	// Mask 'Buffer Ready'.
	static const int R04_MASK_BUF_RDY = 0x08;
	// Mask 'End of sequence'.
	static const int R04_MASK_EOS     = 0x10;
	// Mask Timer2 flag.
	static const int R04_MASK_T2      = 0x20;
	// Mask Timer1 flag.
	static const int R04_MASK_T1      = 0x40;
	// IRQ RESET.
	static const int R04_IRQ_RESET    = 0x80;

	// Bitmask for status register
	static const int STATUS_EOS     = R04_MASK_EOS;
	static const int STATUS_BUF_RDY = R04_MASK_BUF_RDY;
	static const int STATUS_T2      = R04_MASK_T2;
	static const int STATUS_T1      = R04_MASK_T1;

	Y8950(const std::string& name, const DeviceConfig& config,
	      unsigned sampleRam, EmuTime::param time, MSXAudio& audio);
	~Y8950();

	void setEnabled(bool enabled, EmuTime::param time);
	void clearRam();
	void reset(EmuTime::param time);
	void writeReg(byte reg, byte data, EmuTime::param time);
	byte readReg(byte reg, EmuTime::param time);
	byte peekReg(byte reg, EmuTime::param time) const;
	byte readStatus(EmuTime::param time);
	byte peekStatus(EmuTime::param time) const;

	// for ADPCM
	void setStatus(byte flags);
	void resetStatus(byte flags);
	byte peekRawStatus() const;

	template<typename Archive>
	void serialize(Archive& ar, unsigned version);

private:
	class Impl;
	const std::auto_ptr<Impl> pimpl;
};

} // namespace openmsx

#endif
