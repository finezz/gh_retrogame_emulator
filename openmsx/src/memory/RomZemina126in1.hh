// $Id: RomZemina126in1.hh 12527 2012-05-17 17:34:11Z m9710797 $

#ifndef ROMZEMINA126IN1_HH
#define ROMZEMINA126IN1_HH

#include "RomBlocks.hh"

namespace openmsx {

class RomZemina126in1 : public Rom16kBBlocks
{
public:
	RomZemina126in1(const DeviceConfig& config, std::auto_ptr<Rom> rom);

	virtual void reset(EmuTime::param time);
	virtual void writeMem(word address, byte value, EmuTime::param time);
	virtual byte* getWriteCacheLine(word address) const;
};

} // namespace openmsx

#endif
