// $Id: RomHolyQuran2.hh 12551 2012-05-23 17:37:04Z m9710797 $

#ifndef ROMHOLYQURAN2_HH
#define ROMHOLYQURAN2_HH

#include "MSXRom.hh"

namespace openmsx {

class Quran2RomBlocks;

class RomHolyQuran2 : public MSXRom
{
public:
	RomHolyQuran2(const DeviceConfig& config, std::auto_ptr<Rom> rom);

	virtual void reset(EmuTime::param time);
	virtual byte readMem(word address, EmuTime::param time);
	virtual byte peekMem(word address, EmuTime::param time) const;
	virtual void writeMem(word address, byte value, EmuTime::param time);
	virtual const byte* getReadCacheLine(word start) const;
	virtual byte* getWriteCacheLine(word start) const;

	template<typename Archive>
	void serialize(Archive& ar, unsigned version);

private:
	const std::auto_ptr<Quran2RomBlocks> romBlocks;
	const byte* bank[4];
	bool decrypt;

	friend class Quran2RomBlocks;
};

} // namespace openmsx

#endif
