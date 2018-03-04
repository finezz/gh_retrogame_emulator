// $Id: PanasonicRam.hh 12527 2012-05-17 17:34:11Z m9710797 $

#ifndef PANASONICRAM_HH
#define PANASONICRAM_HH

#include "MSXMemoryMapper.hh"

namespace openmsx {

class PanasonicMemory;

class PanasonicRam : public MSXMemoryMapper
{
public:
	explicit PanasonicRam(const DeviceConfig& config);

	virtual void writeMem(word address, byte value, EmuTime::param time);
	virtual byte* getWriteCacheLine(word start) const;

	template<typename Archive>
	void serialize(Archive& ar, unsigned version);

private:
	PanasonicMemory& panasonicMemory;
};

} // namespace openmsx

#endif
