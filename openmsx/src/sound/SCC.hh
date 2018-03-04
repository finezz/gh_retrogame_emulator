// $Id: SCC.hh 12527 2012-05-17 17:34:11Z m9710797 $

#ifndef SCC_HH
#define SCC_HH

#include "ResampledSoundDevice.hh"
#include "Clock.hh"
#include "openmsx.hh"
#include <memory>

namespace openmsx {

class MSXMotherBoard;
class SCCDebuggable;

class SCC : public ResampledSoundDevice
{
public:
	enum ChipMode {SCC_Real, SCC_Compatible, SCC_plusmode};

	SCC(const std::string& name, const DeviceConfig& config,
	    EmuTime::param time, ChipMode mode = SCC_Real);
	virtual ~SCC();

	// interaction with realCartridge
	void powerUp(EmuTime::param time);
	void reset(EmuTime::param time);
	byte readMem(byte address,EmuTime::param time);
	byte peekMem(byte address,EmuTime::param time) const;
	void writeMem(byte address, byte value, EmuTime::param time);
	void setChipMode(ChipMode newMode);

	template<typename Archive>
	void serialize(Archive& ar, unsigned version);

private:
	// SoundDevice
	virtual int getAmplificationFactor() const;
	virtual void generateChannels(int** bufs, unsigned num);

	inline int adjust(signed char wav, byte vol);
	byte readWave(unsigned channel, unsigned address, EmuTime::param time) const;
	void writeWave(unsigned channel, unsigned offset, byte value);
	void setDeformReg(byte value, EmuTime::param time);
	void setDeformRegHelper(byte value);
	void setFreqVol(unsigned address, byte value, EmuTime::param time);
	byte getFreqVol(unsigned address) const;

	static const int CLOCK_FREQ = 3579545;

	friend class SCCDebuggable;
	const std::auto_ptr<SCCDebuggable> debuggable;

	Clock<CLOCK_FREQ> deformTimer;
	ChipMode currentChipMode;

	signed char wave[5][32];
	int volAdjustedWave[5][32];
	unsigned incr[5];
	unsigned count[5];
	unsigned pos[5];
	unsigned period[5];
	unsigned orgPeriod[5];
	int out[5];
	byte volume[5];
	byte ch_enable;

	byte deformValue;
	bool rotate[5];
	bool readOnly[5];
};

} // namespace openmsx

#endif
