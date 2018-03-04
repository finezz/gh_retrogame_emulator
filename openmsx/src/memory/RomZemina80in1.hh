// $Id: RomZemina80in1.hh 12527 2012-05-17 17:34:11Z m9710797 $

#ifndef ROMZEMINA80IN1_HH
#define ROMZEMINA80IN1_HH

#include "RomBlocks.hh"

namespace openmsx {

class RomZemina80in1 : public Rom8kBBlocks
{
public:
	RomZemina80in1(const DeviceConfig& config, std::auto_ptr<Rom> rom);

	virtual void reset(EmuTime::param time);
	virtual void writeMem(word address, byte value, EmuTime::param time);
	virtual byte* getWriteCacheLine(word address) const;
};

} // namespace openmsx

#endif
