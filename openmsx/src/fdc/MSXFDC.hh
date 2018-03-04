// $Id: MSXFDC.hh 12527 2012-05-17 17:34:11Z m9710797 $

#ifndef MSXFDC_HH
#define MSXFDC_HH

#include "MSXDevice.hh"
#include <memory>

namespace openmsx {

class Rom;
class DiskDrive;

class MSXFDC : public MSXDevice
{
public:
	virtual void powerDown(EmuTime::param time);
	virtual byte readMem(word address, EmuTime::param time);
	virtual byte peekMem(word address, EmuTime::param time) const;
	virtual const byte* getReadCacheLine(word start) const;

	template<typename Archive>
	void serialize(Archive& ar, unsigned version);

protected:
	explicit MSXFDC(const DeviceConfig& config);
	virtual ~MSXFDC();

	const std::auto_ptr<Rom> rom;
	std::auto_ptr<DiskDrive> drives[4];
};

REGISTER_BASE_NAME_HELPER(MSXFDC, "FDC");

} // namespace openmsx

#endif
