// $Id: DACSound16S.hh 12631 2012-06-14 20:18:24Z m9710797 $

// This class implements a 16 bit signed DAC

#ifndef DACSOUND16S_HH
#define DACSOUND16S_HH

#include "SoundDevice.hh"
#include "BlipBuffer.hh"

namespace openmsx {

class DACSound16S : public SoundDevice
{
public:
	DACSound16S(string_ref name, string_ref desc,
	            const DeviceConfig& config);
	virtual ~DACSound16S();

	void reset(EmuTime::param time);
	void writeDAC(short value, EmuTime::param time);

	template<typename Archive>
	void serialize(Archive& ar, unsigned version);

private:
	// SoundDevice
	virtual void setOutputRate(unsigned sampleRate);
	virtual void generateChannels(int** bufs, unsigned num);
	virtual bool updateBuffer(unsigned length, int* buffer,
	                          EmuTime::param time);

	BlipBuffer blip;
	short lastWrittenValue;
};

} // namespace openmsx

#endif
