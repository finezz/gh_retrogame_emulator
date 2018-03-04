// $Id: AVTFDC.hh 12729 2012-07-14 17:41:22Z manuelbi $

#ifndef AVTFDC_HH
#define AVTFDC_HH

#include "WD2793BasedFDC.hh"

namespace openmsx {

class AVTFDC : public WD2793BasedFDC
{
public:
	explicit AVTFDC(const DeviceConfig& config);

	virtual byte readIO(word port, EmuTime::param time);
	virtual byte peekIO(word port, EmuTime::param time) const;
	virtual void writeIO(word port, byte value, EmuTime::param time);

	template<typename Archive>
	void serialize(Archive& ar, unsigned version);
};

} // namespace openmsx

#endif
