// $Id: RomKonamiSCC.hh 12527 2012-05-17 17:34:11Z m9710797 $

#ifndef ROMKONAMISCC_HH
#define ROMKONAMISCC_HH

#include "RomBlocks.hh"

namespace openmsx {

class SCC;

class RomKonamiSCC : public Rom8kBBlocks
{
public:
	RomKonamiSCC(const DeviceConfig& config, std::auto_ptr<Rom> rom);
	virtual ~RomKonamiSCC();

	virtual void powerUp(EmuTime::param time);
	virtual void reset(EmuTime::param time);
	virtual byte peekMem(word address, EmuTime::param time) const;
	virtual byte readMem(word address, EmuTime::param time);
	virtual const byte* getReadCacheLine(word address) const;
	virtual void writeMem(word address, byte value, EmuTime::param time);
	virtual byte* getWriteCacheLine(word address) const;

	template<typename Archive>
	void serialize(Archive& ar, unsigned version);

private:
	const std::auto_ptr<SCC> scc;
	bool sccEnabled;
};

} // namespace openmsx

#endif
