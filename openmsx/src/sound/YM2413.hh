// $Id: YM2413.hh 12527 2012-05-17 17:34:11Z m9710797 $

#ifndef YM2413_HH
#define YM2413_HH

#include "ResampledSoundDevice.hh"
#include "EmuTime.hh"
#include "openmsx.hh"
#include <memory>
#include <string>

namespace openmsx {

class YM2413Core;
class YM2413Debuggable;
class MSXMotherBoard;

class YM2413 : public ResampledSoundDevice
{
public:
	YM2413(const std::string& name, const DeviceConfig& config);
	virtual ~YM2413();

	void reset(EmuTime::param time);
	void writeReg(byte reg, byte value, EmuTime::param time);

	template<typename Archive>
	void serialize(Archive& ar, unsigned version);

private:
	// SoundDevice
	virtual void generateChannels(int** bufs, unsigned num);
	virtual int getAmplificationFactor() const;

	const std::auto_ptr<YM2413Core> core;
	const std::auto_ptr<YM2413Debuggable> debuggable;
	friend class YM2413Debuggable;
};

} // namespace openmsx

#endif

