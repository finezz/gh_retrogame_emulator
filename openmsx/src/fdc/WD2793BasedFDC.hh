// $Id: WD2793BasedFDC.hh 12527 2012-05-17 17:34:11Z m9710797 $

#ifndef WD2793BASEDFDC_HH
#define WD2793BASEDFDC_HH

#include "MSXFDC.hh"
#include <memory>

namespace openmsx {

class DriveMultiplexer;
class WD2793;

class WD2793BasedFDC : public MSXFDC
{
public:
	virtual void reset(EmuTime::param time);

	template<typename Archive>
	void serialize(Archive& ar, unsigned version);

protected:
	explicit WD2793BasedFDC(const DeviceConfig& config);
	virtual ~WD2793BasedFDC();

	const std::auto_ptr<DriveMultiplexer> multiplexer;
	const std::auto_ptr<WD2793> controller;
};

REGISTER_BASE_NAME_HELPER(WD2793BasedFDC, "WD2793BasedFDC");

} // namespace openmsx

#endif
