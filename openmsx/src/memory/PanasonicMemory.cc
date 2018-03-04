// $Id: PanasonicMemory.cc 12528 2012-05-17 17:36:10Z m9710797 $

#include "PanasonicMemory.hh"
#include "MSXMotherBoard.hh"
#include "MSXCPU.hh"
#include "Ram.hh"
#include "Rom.hh"
#include "DeviceConfig.hh"
#include "HardwareConfig.hh"
#include "XMLElement.hh"
#include "MSXException.hh"

namespace openmsx {

static Rom* createRom(MSXMotherBoard& motherBoard)
{
	const XMLElement* elem = motherBoard.getMachineConfig()->
	                      getConfig().findChild("PanasonicRom");
	if (!elem) return NULL;

	const HardwareConfig* hwConf = motherBoard.getMachineConfig();
	assert(hwConf);
	return new Rom("PanasonicRom", "Turbor-R main ROM",
	               DeviceConfig(*hwConf, *elem));
}

PanasonicMemory::PanasonicMemory(MSXMotherBoard& motherBoard)
	: msxcpu(motherBoard.getCPU())
	, rom(createRom(motherBoard))
	, ram(NULL), dram(false)
{
}

PanasonicMemory::~PanasonicMemory()
{
}

void PanasonicMemory::registerRam(Ram& ram_)
{
	ram = &ram_[0];
	ramSize = ram_.getSize();
}

const byte* PanasonicMemory::getRomBlock(unsigned block)
{
	if (!rom.get()) {
		throw MSXException("Missing PanasonicRom.");
	}
	if (dram &&
	    (((0x28 <= block) && (block < 0x2C)) ||
	     ((0x38 <= block) && (block < 0x3C)))) {
		assert(ram);
		unsigned offset = (block & 0x03) * 0x2000;
		unsigned ramOffset = (block < 0x30) ? ramSize - 0x10000 :
		                                      ramSize - 0x08000;
		return ram + ramOffset + offset;
	} else {
		unsigned offset = block * 0x2000;
		if (offset >= rom->getSize()) {
			offset &= rom->getSize() - 1;
		}
		return &(*rom)[offset];
	}
}

const byte* PanasonicMemory::getRomRange(unsigned first, unsigned last)
{
	if (!rom.get()) {
		throw MSXException("Missing PanasonicRom.");
	}
	if (last < first) {
		throw MSXException("Error in config file: firstblock must "
		                   "be smaller than lastblock");
	}
	unsigned start =  first     * 0x2000;
	if (start >= rom->getSize()) {
		throw MSXException("Error in config file: firstblock lies "
		                   "outside of rom image.");
	}
	unsigned stop  = (last + 1) * 0x2000;
	if (stop > rom->getSize()) {
		throw MSXException("Error in config file: lastblock lies "
		                   "outside of rom image.");
	}
	return &(*rom)[start];
}

byte* PanasonicMemory::getRamBlock(unsigned block)
{
	if (!ram) return NULL;

	unsigned offset = block * 0x2000;
	if (offset >= ramSize) {
		offset &= ramSize - 1;
	}
	return ram + offset;
}
unsigned PanasonicMemory::getRamSize() const
{
	if (!ram) return 0;

	return ramSize;
}

void PanasonicMemory::setDRAM(bool dram_)
{
	if (dram_ != dram) {
		dram = dram_;
		msxcpu.invalidateMemCache(0x0000, 0x10000);
	}
}

bool PanasonicMemory::isWritable(unsigned address) const
{
	return !dram || (address < (ramSize - 0x10000));
}

} // namespace openmsx
