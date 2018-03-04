// $Id: RomZemina90in1.hh 12527 2012-05-17 17:34:11Z m9710797 $

#ifndef ROMZEMINA90IN1_HH
#define ROMZEMINA90IN1_HH

#include "RomBlocks.hh"

namespace openmsx {

class RomZemina90in1 : public Rom8kBBlocks
{
public:
	RomZemina90in1(const DeviceConfig& config, std::auto_ptr<Rom> rom);
	virtual ~RomZemina90in1();

	virtual void reset(EmuTime::param time);
	void writeIO(word port, byte value, EmuTime::param time);
	virtual byte* getWriteCacheLine(word address) const;
};

} // namespace openmsx

#endif
