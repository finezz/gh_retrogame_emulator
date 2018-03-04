// $Id: MegaSCSI.hh 12547 2012-05-22 19:38:18Z bifimsx $

#ifndef MEGASCSI_HH
#define MEGASCSI_HH

#include "MSXDevice.hh"
#include <memory>

namespace openmsx {

class MB89352;
class SRAM;
class RomBlockDebuggable;

class MegaSCSI : public MSXDevice
{
public:
	explicit MegaSCSI(const DeviceConfig& config);
	virtual ~MegaSCSI();

	virtual void reset(EmuTime::param time);

	virtual byte readMem(word address, EmuTime::param time);
	virtual byte peekmem(word address, EmuTime::param time) const;
	virtual void writeMem(word address, byte value, EmuTime::param time);
	virtual const byte* getReadCacheLine(word start) const;
	virtual byte* getWriteCacheLine(word start) const;

	template<typename Archive>
	void serialize(Archive& ar, unsigned version);

private:
	void setSRAM(unsigned region, byte block);

	const std::auto_ptr<MB89352> mb89352;
	const std::auto_ptr<SRAM> sram;
	const std::auto_ptr<RomBlockDebuggable> romBlockDebug;

	bool isWriteable[4]; // which region is readonly?
	byte mapped[4]; // SPC block mapped in this region?
	const byte blockMask;
};

} // namespace openmsx

#endif
