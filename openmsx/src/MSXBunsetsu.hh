// $Id: MSXBunsetsu.hh 12527 2012-05-17 17:34:11Z m9710797 $

#ifndef MSXBUNSETSU_HH
#define MSXBUNSETSU_HH

#include "MSXDevice.hh"
#include <memory>

namespace openmsx {

class Rom;

class MSXBunsetsu : public MSXDevice
{
public:
	explicit MSXBunsetsu(const DeviceConfig& DeviceConfig);
	virtual ~MSXBunsetsu();

	virtual void reset(EmuTime::param time);

	virtual byte readMem(word address, EmuTime::param time);
	virtual void writeMem(word address, byte value, EmuTime::param time);
	virtual const byte* getReadCacheLine(word start) const;
	virtual byte* getWriteCacheLine(word start) const;

	template<typename Archive>
	void serialize(Archive& ar, unsigned version);

private:
	const std::auto_ptr<Rom> bunsetsuRom;
	const std::auto_ptr<Rom> jisyoRom;
	unsigned jisyoAddress;
};

} // namespace openmsx

#endif
