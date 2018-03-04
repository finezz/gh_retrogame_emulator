// $Id: MSXRom.cc 12770 2012-08-02 20:58:32Z manuelbi $

#include "MSXRom.hh"
#include "Rom.hh"
#include "XMLElement.hh"
#include "TclObject.hh"

namespace openmsx {

MSXRom::MSXRom(const DeviceConfig& config, std::auto_ptr<Rom> rom_)
	: MSXDevice(config, rom_->getName())
	, rom(rom_)
{
}

MSXRom::~MSXRom()
{
}

void MSXRom::writeMem(word /*address*/, byte /*value*/, EmuTime::param /*time*/)
{
	// nothing
}

byte* MSXRom::getWriteCacheLine(word /*address*/) const
{
	return unmappedWrite;
}

void MSXRom::getExtraDeviceInfo(TclObject& result) const
{
	//
	// TODO: change all of this to return a dict!
	//
	// Add detected rom type. This value is guaranteed to be stored in
	// the device config (and 'auto' is already changed to actual type).
	const XMLElement* mapper = getDeviceConfig().findChild("mappertype");
	assert(mapper);
	result.addListElement(mapper->getData());

	// add sha1sum, to be able to get a unique key for this ROM device,
	// so that it can be used to look up things in databases
	result.addListElement(rom->getOriginalSHA1().toString());

	// add original filename, e.g. to be able to see whether it comes
	// from a system_rom pool
	result.addListElement(rom->getFilename());
}

} // namespace openmsx
