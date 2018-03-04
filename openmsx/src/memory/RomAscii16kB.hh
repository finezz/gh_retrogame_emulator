// $Id: RomAscii16kB.hh 12527 2012-05-17 17:34:11Z m9710797 $

#ifndef ROMASCII16KB_HH
#define ROMASCII16KB_HH

#include "RomBlocks.hh"

namespace openmsx {

class RomAscii16kB : public Rom16kBBlocks
{
public:
	RomAscii16kB(const DeviceConfig& config, std::auto_ptr<Rom> rom);

	virtual void reset(EmuTime::param time);
	virtual void writeMem(word address, byte value, EmuTime::param time);
	virtual byte* getWriteCacheLine(word address) const;
};

REGISTER_BASE_CLASS(RomAscii16kB, "RomAscii16kB");

} // namespace openmsx

#endif
