// $Id: MSXKanji.hh 12527 2012-05-17 17:34:11Z m9710797 $

#ifndef MSXKANJI_HH
#define MSXKANJI_HH

#include "MSXDevice.hh"
#include <memory>

namespace openmsx {

class Rom;

class MSXKanji : public MSXDevice
{
public:
	explicit MSXKanji(const DeviceConfig& config);
	virtual ~MSXKanji();

	virtual byte readIO(word port, EmuTime::param time);
	virtual byte peekIO(word port, EmuTime::param time) const;
	virtual void writeIO(word port, byte value, EmuTime::param time);
	virtual void reset(EmuTime::param time);

	template<typename Archive>
	void serialize(Archive& ar, unsigned version);

private:
	const std::auto_ptr<Rom> rom;
	unsigned adr1, adr2;
	const bool isLascom;
};

} // namespace openmsx

#endif
