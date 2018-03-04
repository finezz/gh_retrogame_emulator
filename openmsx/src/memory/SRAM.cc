// $Id: SRAM.cc 12740 2012-07-19 18:18:29Z m9710797 $

#include "SRAM.hh"
#include "DeviceConfig.hh"
#include "File.hh"
#include "FileContext.hh"
#include "FileException.hh"
#include "FileNotFoundException.hh"
#include "MSXMotherBoard.hh"
#include "Reactor.hh"
#include "CliComm.hh"
#include "AlarmEvent.hh"
#include "serialize.hh"
#include "openmsx.hh"
#include "vla.hh"
#include <cstring>

using std::string;

namespace openmsx {

// class SRAM

/* Creates a SRAM that is not loaded from or saved to a file.
 * The only reason to use this (instead of a plain Ram object) is when you
 * dynamically need to decide whether load/save is needed.
 */
SRAM::SRAM(const std::string& name, const std::string& description,
           int size, const DeviceConfig& config, DontLoad)
	: ram(config, name, description, size)
	, header(NULL) // not used
	, sramSync(new AlarmEvent(config.getReactor().getEventDistributor(),
	                          *this, OPENMSX_SAVE_SRAM)) // used, but not needed
{
}

SRAM::SRAM(const string& name, int size,
           const DeviceConfig& config_, const char* header_, bool* loaded)
	: config(config_)
	, ram(config, name, "sram", size)
	, header(header_)
	, sramSync(new AlarmEvent(config.getReactor().getEventDistributor(),
	                          *this, OPENMSX_SAVE_SRAM))
{
	load(loaded);
}

SRAM::SRAM(const string& name, const string& description, int size,
	   const DeviceConfig& config_, const char* header_, bool* loaded)
	: config(config_)
	, ram(config, name, description, size)
	, header(header_)
	, sramSync(new AlarmEvent(config.getReactor().getEventDistributor(),
	                          *this, OPENMSX_SAVE_SRAM))
{
	load(loaded);
}

SRAM::~SRAM()
{
	save();
}

void SRAM::write(unsigned addr, byte value)
{
	if (!sramSync->pending()) {
		sramSync->schedule(5000000); // sync to disk after 5s
	}
	assert(addr < getSize());
	ram[addr] = value;
}

void SRAM::memset(unsigned addr, byte c, unsigned size)
{
	if (!sramSync->pending()) {
		sramSync->schedule(5000000); // sync to disk after 5s
	}
	assert((addr + size) <= getSize());
	::memset(&ram[addr], c, size);
}

void SRAM::load(bool* loaded)
{
	assert(config.getXML());
	if (loaded) *loaded = false;
	const string& filename = config.getChildData("sramname");
	try {
		bool headerOk = true;
		File file(config.getFileContext().resolveCreate(filename),
			  File::LOAD_PERSISTENT);
		if (header) {
			int length = int(strlen(header));
			VLA(char, temp, length);
			file.read(temp, length);
			if (strncmp(temp, header, length) != 0) {
				headerOk = false;
			}
		}
		if (headerOk) {
			file.read(&ram[0], getSize());
			if (loaded) *loaded = true;
		} else {
			config.getCliComm().printWarning(
				"Warning no correct SRAM file: " + filename);
		}
	} catch (FileNotFoundException& /*e*/) {
		config.getCliComm().printInfo(
			"SRAM file " + filename + " not found" +
			", assuming blank SRAM content.");
	} catch (FileException& e) {
		config.getCliComm().printWarning(
			"Couldn't load SRAM " + filename +
			" (" + e.getMessage() + ").");
	}
}

void SRAM::save()
{
	if (!config.getXML()) return;
	const string& filename = config.getChildData("sramname");
	try {
		File file(config.getFileContext().resolveCreate(filename),
			  File::SAVE_PERSISTENT);
		if (header) {
			int length = int(strlen(header));
			file.write(header, length);
		}
		file.write(&ram[0], getSize());
	} catch (FileException& e) {
		config.getCliComm().printWarning(
			"Couldn't save SRAM " + filename +
			" (" + e.getMessage() + ").");
	}
}

int SRAM::signalEvent(const shared_ptr<const Event>& event)
{
	// Event is received by all SRAM instances, so it will trigger a save
	// of all SRAMs. Could be optimized if it turns out to be a problem.
	assert(event->getType() == OPENMSX_SAVE_SRAM);
	(void)event;

	save();
	return 0;
}

template<typename Archive>
void SRAM::serialize(Archive& ar, unsigned /*version*/)
{
	ar.serialize("ram", ram);
}
INSTANTIATE_SERIALIZE_METHODS(SRAM);

} // namespace openmsx
