// $Id: RomPanasonic.hh 12527 2012-05-17 17:34:11Z m9710797 $

#ifndef ROMPANASONIC_HH
#define ROMPANASONIC_HH

#include "RomBlocks.hh"

namespace openmsx {

class PanasonicMemory;

class RomPanasonic : public Rom8kBBlocks
{
public:
	RomPanasonic(const DeviceConfig& config, std::auto_ptr<Rom> rom);
	virtual ~RomPanasonic();

	virtual void reset(EmuTime::param time);
	virtual byte peekMem(word address, EmuTime::param time) const;
	virtual byte readMem(word address, EmuTime::param time);
	virtual const byte* getReadCacheLine(word address) const;
	virtual void writeMem(word address, byte value, EmuTime::param time);
	virtual byte* getWriteCacheLine(word address) const;

	template<typename Archive>
	void serialize(Archive& ar, unsigned version);

private:
	void changeBank(byte region, int bank);

	PanasonicMemory& panasonicMem;
	int maxSRAMBank;

	int bankSelect[8];
	byte control;
};

} // namespace openmsx

#endif
