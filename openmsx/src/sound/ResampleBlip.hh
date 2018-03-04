// $Id: ResampleBlip.hh 12241 2011-08-27 14:58:39Z m9710797 $

#ifndef RESAMPLEBLIP_HH
#define RESAMPLEBLIP_HH

#include "ResampleAlgo.hh"
#include "BlipBuffer.hh"
#include "DynamicClock.hh"

namespace openmsx {

class ResampledSoundDevice;

template <unsigned CHANNELS>
class ResampleBlip : public ResampleAlgo
{
public:
	ResampleBlip(ResampledSoundDevice& input,
	             const DynamicClock& hostClock, unsigned emuSampleRate);

	virtual bool generateOutput(int* dataOut, unsigned num,
	                            EmuTime::param time);

private:
	BlipBuffer blip[CHANNELS];
	ResampledSoundDevice& input;
	const DynamicClock& hostClock; // time of the last host-sample,
	                               //    ticks once per host sample
	DynamicClock emuClock;         // time of the last emu-sample,
	                               //    ticks once per emu-sample
	typedef FixedPoint<16> FP;
	const FP step;
	int lastInput[CHANNELS];
};

} // namespace openmsx

#endif
