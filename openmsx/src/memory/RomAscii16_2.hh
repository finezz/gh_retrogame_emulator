// $Id: RomAscii16_2.hh 12527 2012-05-17 17:34:11Z m9710797 $

#ifndef ROMASCII16_2_HH
#define ROMASCII16_2_HH

#include "RomAscii16kB.hh"

namespace openmsx {

class RomAscii16_2 : public RomAscii16kB
{
public:
	RomAscii16_2(const DeviceConfig& config, std::auto_ptr<Rom> rom);
	virtual ~RomAscii16_2();

	virtual void reset(EmuTime::param time);
	virtual byte readMem(word address, EmuTime::param time);
	virtual const byte* getReadCacheLine(word address) const;
	virtual void writeMem(word address, byte value, EmuTime::param time);
	virtual byte* getWriteCacheLine(word address) const;

	template<typename Archive>
	void serialize(Archive& ar, unsigned version);

private:
	byte sramEnabled;
};

} // namespace openmsx

#endif
