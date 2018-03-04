// $Id: VictorFDC.hh 12527 2012-05-17 17:34:11Z m9710797 $

#ifndef VICTORFDC_HH
#define VICTORFDC_HH

#include "WD2793BasedFDC.hh"

namespace openmsx {

class VictorFDC : public WD2793BasedFDC
{
public:
	explicit VictorFDC(const DeviceConfig& config);

	virtual void reset(EmuTime::param time);
	virtual byte readMem(word address, EmuTime::param time);
	virtual byte peekMem(word address, EmuTime::param time) const;
	virtual void writeMem(word address, byte value, EmuTime::param time);
	virtual const byte* getReadCacheLine(word start) const;
	virtual byte* getWriteCacheLine(word address) const;

	template<typename Archive>
	void serialize(Archive& ar, unsigned version);

private:
	byte driveControls;
};

} // namespace openmsx

#endif
