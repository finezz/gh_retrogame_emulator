// $Id: RomMSXDOS2.hh 12527 2012-05-17 17:34:11Z m9710797 $

#ifndef ROMMSXDOS2_HH
#define ROMMSXDOS2_HH

#include "RomBlocks.hh"

namespace openmsx {

class RomMSXDOS2 : public Rom16kBBlocks
{
public:
	RomMSXDOS2(const DeviceConfig& config, std::auto_ptr<Rom> rom);

	virtual void reset(EmuTime::param time);
	virtual void writeMem(word address, byte value, EmuTime::param time);
	virtual byte* getWriteCacheLine(word address) const;

private:
	const byte range;
};

} // namespace openmsx

#endif
