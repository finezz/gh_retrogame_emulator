// $Id: MSXDevice.cc 12627 2012-06-14 20:14:52Z m9710797 $

#include "MSXDevice.hh"
#include "XMLElement.hh"
#include "MSXMotherBoard.hh"
#include "HardwareConfig.hh"
#include "CartridgeSlotManager.hh"
#include "MSXCPUInterface.hh"
#include "MSXCPU.hh"
#include "CacheLine.hh"
#include "TclObject.hh"
#include "StringOp.hh"
#include "MSXException.hh"
#include "serialize.hh"
#include "unreachable.hh"
#include <set>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <iterator> // for back_inserter

using std::string;
using std::vector;

namespace openmsx {

byte MSXDevice::unmappedRead[0x10000];
byte MSXDevice::unmappedWrite[0x10000];


MSXDevice::MSXDevice(const DeviceConfig& config, const string& name)
	: deviceConfig(config)
{
	initName(name);
}

MSXDevice::MSXDevice(const DeviceConfig& config)
	: deviceConfig(config)
{
	initName(getDeviceConfig().getAttribute("id"));
}

void MSXDevice::initName(const string& name)
{
	deviceName = name;
	if (getMotherBoard().findDevice(deviceName)) {
		unsigned n = 0;
		do {
			deviceName = StringOp::Builder() << name << " (" << ++n << ')';
		} while (getMotherBoard().findDevice(deviceName));
	}
}

void MSXDevice::init()
{
	staticInit();

	lockDevices();
	registerSlots();
	registerPorts();
}

MSXDevice::~MSXDevice()
{
	unregisterPorts();
	unregisterSlots();
	unlockDevices();
	assert(referencedBy.empty());
}

void MSXDevice::staticInit()
{
	static bool alreadyInit = false;
	if (alreadyInit) return;
	alreadyInit = true;

	memset(unmappedRead, 0xFF, sizeof(unmappedRead));
}

const HardwareConfig& MSXDevice::getHardwareConfig() const
{
	return deviceConfig.getHardwareConfig();
}

MSXMotherBoard& MSXDevice::getMotherBoard() const
{
	return getHardwareConfig().getMotherBoard();
}

const XMLElement& MSXDevice::getDeviceConfig() const
{
	return *deviceConfig.getXML();
}

const DeviceConfig& MSXDevice::getDeviceConfig2() const
{
	return deviceConfig;
}

void MSXDevice::testRemove(const Devices& alreadyRemoved) const
{
	std::set<MSXDevice*> all    (referencedBy  .begin(), referencedBy  .end());
	std::set<MSXDevice*> removed(alreadyRemoved.begin(), alreadyRemoved.end());
	Devices rest;
	set_difference(all.begin(), all.end(), removed.begin(), removed.end(),
	               back_inserter(rest));
	if (!rest.empty()) {
		StringOp::Builder msg;
		msg << "Still in use by ";
		for (Devices::const_iterator it = rest.begin();
		     it != rest.end(); ++it) {
			msg << (*it)->getName() << ' ';
		}
		throw MSXException(msg);
	}
}

void MSXDevice::lockDevices()
{
	// This code can only handle backward references: the thing that is
	// referenced must already be instantiated, we don't try to change the
	// instantiation order. For the moment this is good enough (only ADVRAM
	// (an extension) uses it to refer to the VDP (inside a machine)). If
	// needed we can implement something more sophisticated later without
	// changing the format of the config files.
	XMLElement::Children refConfigs;
	getDeviceConfig().getChildren("device", refConfigs);
	for (XMLElement::Children::const_iterator it = refConfigs.begin();
	     it != refConfigs.end(); ++it) {
		string name = (*it)->getAttribute("idref");
		MSXDevice* dev = getMotherBoard().findDevice(name);
		if (!dev) {
			throw MSXException(
				"Unsatisfied dependency: '" + getName() +
				"' depends on unavailable device '" +
				name + "'.");
		}
		references.push_back(dev);
		dev->referencedBy.push_back(this);
	}
}

void MSXDevice::unlockDevices()
{
	for (Devices::const_iterator it = references.begin();
	     it != references.end(); ++it) {
		Devices::iterator it2 = find((*it)->referencedBy.begin(),
		                             (*it)->referencedBy.end(),
		                             this);
		assert(it2 != (*it)->referencedBy.end());
		(*it)->referencedBy.erase(it2);
	}
}

const MSXDevice::Devices& MSXDevice::getReferences() const
{
	// init() must already be called
	return references;
}

EmuTime::param MSXDevice::getCurrentTime() const
{
	return getMotherBoard().getCurrentTime();
}
MSXCPU& MSXDevice::getCPU() const
{
	return getMotherBoard().getCPU();
}
MSXCPUInterface& MSXDevice::getCPUInterface() const
{
	return getMotherBoard().getCPUInterface();
}
Scheduler& MSXDevice::getScheduler() const
{
	return getMotherBoard().getScheduler();
}
CliComm& MSXDevice::getCliComm() const
{
	return getMotherBoard().getMSXCliComm();
}
Reactor& MSXDevice::getReactor() const
{
	return getMotherBoard().getReactor();
}
CommandController& MSXDevice::getCommandController() const
{
	return getMotherBoard().getCommandController();
}
LedStatus& MSXDevice::getLedStatus() const
{
	return getMotherBoard().getLedStatus();
}
PluggingController& MSXDevice::getPluggingController() const
{
	return getMotherBoard().getPluggingController();
}

void MSXDevice::registerSlots()
{
	MemRegions tmpMemRegions;
	XMLElement::Children memConfigs;
	getDeviceConfig().getChildren("mem", memConfigs);
	for (XMLElement::Children::const_iterator it = memConfigs.begin();
	     it != memConfigs.end(); ++it) {
		unsigned base = (*it)->getAttributeAsInt("base");
		unsigned size = (*it)->getAttributeAsInt("size");
		if ((base >= 0x10000) || (size > 0x10000)) {
			throw MSXException(
				"Invalid memory specification for device " +
				getName() + " should be in range "
				"[0x0000,0x10000).");
		}
		tmpMemRegions.push_back(std::make_pair(base, size));
	}
	if (tmpMemRegions.empty()) {
		return;
	}

	// find primary and secondary slot specification
	CartridgeSlotManager& slotManager = getMotherBoard().getSlotManager();
	XMLElement* primaryConfig   = getDeviceConfig2().getPrimary();
	XMLElement* secondaryConfig = getDeviceConfig2().getSecondary();
	if (primaryConfig) {
		ps = slotManager.getSlotNum(primaryConfig->getAttribute("slot"));
	} else {
		throw MSXException("Invalid memory specification");
	}
	if (secondaryConfig) {
		ss = slotManager.getSlotNum(secondaryConfig->getAttribute("slot"));
	} else {
		ss = 0;
	}

	// This is only for backwards compatibility: in the past we added extra
	// attributes "primary_slot" and "secondary_slot" (in each MSXDevice
	// config) instead of changing the 'any' value of the slot attribute of
	// the (possibly shared) <primary> and <secondary> tags. When loading
	// an old savestate these tags can still occur, so keep this code. Also
	// remove these attributes to convert to the new format.
	const XMLElement& config = getDeviceConfig();
	if (config.hasAttribute("primary_slot")) {
		XMLElement& mutableConfig = const_cast<XMLElement&>(config);
		const string& primSlot = config.getAttribute("primary_slot");
		ps = slotManager.getSlotNum(primSlot);
		mutableConfig.removeAttribute("primary_slot");
		if (config.hasAttribute("secondary_slot")) {
			const string& secondSlot = config.getAttribute("secondary_slot");
			ss = slotManager.getSlotNum(secondSlot);
			mutableConfig.removeAttribute("secondary_slot");
		}
	}

	// decode special values for 'ss'
	if ((-128 <= ss) && (ss < 0)) {
		if ((0 <= ps) && (ps < 4) &&
		    getCPUInterface().isExpanded(ps)) {
			ss += 128;
		} else {
			ss = 0;
		}
	}

	// decode special values for 'ps'
	if (ps == -256) {
		slotManager.getAnyFreeSlot(ps, ss);
	} else if (ps < 0) {
		// specified slot by name (carta, cartb, ...)
		slotManager.getSpecificSlot(-ps - 1, ps, ss);
	} else {
		// numerical specified slot (0, 1, 2, 3)
	}

	if (!getCPUInterface().isExpanded(ps)) {
		ss = -1;
	}

	// Store actual slot in config. This has two purposes:
	//  - Make sure that devices that are grouped under the same
	//    <primary>/<secondary> tags actually use the same slot. (This
	//    matters when the value of some of the slot attributes is "any").
	//  - Fix the slot number so that it remains the same after a
	//    savestate/loadstate.
	assert(primaryConfig);
	primaryConfig->setAttribute("slot", StringOp::toString(ps));
	if (secondaryConfig) {
		string slot = (ss == -1) ? "X" : StringOp::toString(ss);
		secondaryConfig->setAttribute("slot", slot);
	} else {
		if (ss != -1) {
			throw MSXException(
				"Missing <secondary> tag for device" +
				getName());
		}
	}

	int logicalSS = (ss == -1) ? 0 : ss;
	for (MemRegions::const_iterator it = tmpMemRegions.begin();
	     it != tmpMemRegions.end(); ++it) {
		getCPUInterface().registerMemDevice(
			*this, ps, logicalSS, it->first, it->second);
		memRegions.push_back(*it);
	}

	// Mark the slot as 'in-use' so that future searches for free external
	// slots don't return this slot anymore. If the slot was not an
	// external slot, this call has no effect. Multiple MSXDevices from the
	// same extension (the same HardwareConfig) can all allocate the same
	// slot (later they should also all free this slot).
	slotManager.allocateSlot(ps, ss, getHardwareConfig());
}

void MSXDevice::unregisterSlots()
{
	if (memRegions.empty()) return;

	int logicalSS = (ss == -1) ? 0 : ss;
	for (MemRegions::const_iterator it = memRegions.begin();
	     it != memRegions.end(); ++it) {
		getCPUInterface().unregisterMemDevice(
			*this, ps, logicalSS, it->first, it->second);
	}

	// See comments above about allocateSlot() for more details:
	//  - has no effect for non-external slots
	//  - can be called multiple times for the same slot
	getMotherBoard().getSlotManager().freeSlot(ps, ss, getHardwareConfig());
}

void MSXDevice::getVisibleMemRegion(unsigned& base, unsigned& size) const
{
	// init() must already be called
	if (memRegions.empty()) {
		base = 0;
		size = 0;
		return;
	}
	MemRegions::const_iterator it = memRegions.begin();
	unsigned lowest  = it->first;
	unsigned highest = it->first + it->second;
	for (++it; it != memRegions.end(); ++it) {
		lowest  = std::min(lowest,  it->first);
		highest = std::max(highest, it->first + it->second);
	}
	assert(lowest <= highest);
	base = lowest;
	size = highest - lowest;
}

void MSXDevice::registerPorts()
{
	XMLElement::Children ios;
	getDeviceConfig().getChildren("io", ios);
	for (XMLElement::Children::const_iterator it = ios.begin();
	     it != ios.end(); ++it) {
		unsigned base = StringOp::stringToInt((*it)->getAttribute("base"));
		unsigned num  = StringOp::stringToInt((*it)->getAttribute("num"));
		string_ref type = (*it)->getAttribute("type", "IO");
		if (((base + num) > 256) || (num == 0) ||
		    ((type != "I") && (type != "O") && (type != "IO"))) {
			throw MSXException("Invalid IO port specification");
		}
		for (unsigned port = base; port < base + num; ++port) {
			if ((type == "I") || (type == "IO")) {
				getCPUInterface().register_IO_In(port, this);
				inPorts.push_back(port);
			}
			if ((type == "O") || (type == "IO")) {
				getCPUInterface().register_IO_Out(port, this);
				outPorts.push_back(port);
			}
		}
	}
}

void MSXDevice::unregisterPorts()
{
	for (vector<byte>::iterator it = inPorts.begin();
	     it != inPorts.end(); ++it) {
		getCPUInterface().unregister_IO_In(*it, this);
	}
	for (vector<byte>::iterator it = outPorts.begin();
	     it != outPorts.end(); ++it) {
		getCPUInterface().unregister_IO_Out(*it, this);
	}
}


void MSXDevice::reset(EmuTime::param /*time*/)
{
	// nothing
}

byte MSXDevice::readIRQVector()
{
	return 0xFF;
}

void MSXDevice::powerDown(EmuTime::param /*time*/)
{
	// nothing
}

void MSXDevice::powerUp(EmuTime::param time)
{
	reset(time);
}

string MSXDevice::getName() const
{
	return deviceName;
}

void MSXDevice::getDeviceInfo(TclObject& result) const
{
	result.addListElement(getDeviceConfig().getName());
	getExtraDeviceInfo(result);
}

void MSXDevice::getExtraDeviceInfo(TclObject& /*result*/) const
{
	// nothing
}


byte MSXDevice::readIO(word port, EmuTime::param /*time*/)
{
	(void)port;
	PRT_DEBUG("MSXDevice::readIO (0x" << std::hex << int(port & 0xFF)
	          << std::dec << ") : No device implementation.");
	return 0xFF;
}

void MSXDevice::writeIO(word port, byte value, EmuTime::param /*time*/)
{
	(void)port;
	(void)value;
	PRT_DEBUG("MSXDevice::writeIO(port 0x" << std::hex << int(port & 0xFF)
	          << std::dec << ",value " << int(value)
	          << ") : No device implementation.");
	// do nothing
}

byte MSXDevice::peekIO(word /*port*/, EmuTime::param /*time*/) const
{
	return 0xFF;
}


byte MSXDevice::readMem(word address, EmuTime::param /*time*/)
{
	(void)address;
	PRT_DEBUG("MSXDevice: read from unmapped memory " << std::hex <<
	          int(address) << std::dec);
	return 0xFF;
}

const byte* MSXDevice::getReadCacheLine(word /*start*/) const
{
	return NULL; // uncacheable
}

void MSXDevice::writeMem(word address, byte /*value*/,
                            EmuTime::param /*time*/)
{
	(void)address;
	PRT_DEBUG("MSXDevice: write to unmapped memory " << std::hex <<
	          int(address) << std::dec);
	// do nothing
}

byte MSXDevice::peekMem(word address, EmuTime::param /*time*/) const
{
	word base = address & CacheLine::HIGH;
	if (const byte* cache = getReadCacheLine(base)) {
		word offset = address & CacheLine::LOW;
		return cache[offset];
	} else {
		PRT_DEBUG("MSXDevice: peek not supported for this device");
		return 0xFF;
	}
}

void MSXDevice::globalWrite(word /*address*/, byte /*value*/,
                            EmuTime::param /*time*/)
{
	UNREACHABLE;
}

byte* MSXDevice::getWriteCacheLine(word /*start*/) const
{
	return NULL; // uncacheable
}

void MSXDevice::invalidateMemCache(word start, unsigned size)
{
	getCPU().invalidateMemCache(start, size);
}

template<typename Archive>
void MSXDevice::serialize(Archive& ar, unsigned /*version*/)
{
	// When this method is called, the method init() has already been
	// called (thus also registerSlots() and registerPorts()).
	ar.serialize("name", deviceName);
}
INSTANTIATE_SERIALIZE_METHODS(MSXDevice);

} // namespace openmsx
