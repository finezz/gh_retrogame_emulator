// $Id: RomManbow2.hh 12547 2012-05-22 19:38:18Z bifimsx $

#ifndef ROMMANBOW2_HH
#define ROMMANBOW2_HH

#include "MSXRom.hh"
#include "RomTypes.hh"
#include "serialize_meta.hh"
#include <memory>

namespace openmsx {

class SCC;
class AY8910;
class AmdFlash;
class RomBlockDebuggable;

class RomManbow2 : public MSXRom
{
public:
	RomManbow2(const DeviceConfig& config, std::auto_ptr<Rom> rom,
	           RomType type);
	virtual ~RomManbow2();

	virtual void powerUp(EmuTime::param time);
	virtual void reset(EmuTime::param time);
	virtual byte peekMem(word address, EmuTime::param time) const;
	virtual byte readMem(word address, EmuTime::param time);
	virtual void writeMem(word address, byte value, EmuTime::param time);
	virtual const byte* getReadCacheLine(word address) const;
	virtual byte* getWriteCacheLine(word address) const;

	virtual byte readIO(word port, EmuTime::param time);
	virtual byte peekIO(word port, EmuTime::param time) const;
	virtual void writeIO(word port, byte value, EmuTime::param time);

	template<typename Archive>
	void serialize(Archive& ar, unsigned version);

private:
	void setRom(unsigned region, unsigned block);

	const std::auto_ptr<SCC> scc;
	const std::auto_ptr<AY8910> psg;
	const std::auto_ptr<AmdFlash> flash;
	const std::auto_ptr<RomBlockDebuggable> romBlockDebug;
	byte psgLatch;
	byte bank[4];
	bool sccEnabled;
};
SERIALIZE_CLASS_VERSION(RomManbow2, 2);

} // namespace openmsx

#endif
