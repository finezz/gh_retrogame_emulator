// $Id: HardwareConfig.cc 12617 2012-06-14 20:08:44Z m9710797 $

#include "HardwareConfig.hh"
#include "XMLLoader.hh"
#include "XMLException.hh"
#include "DeviceConfig.hh"
#include "XMLElement.hh"
#include "LocalFileReference.hh"
#include "FileContext.hh"
#include "FileOperations.hh"
#include "MSXMotherBoard.hh"
#include "CartridgeSlotManager.hh"
#include "MSXCPUInterface.hh"
#include "DeviceFactory.hh"
#include "CliComm.hh"
#include "serialize.hh"
#include "serialize_stl.hh"
#include "StringOp.hh"
#include "ref.hh"
#include "unreachable.hh"
#include <cassert>

#include <iostream>

using std::string;
using std::vector;
using std::auto_ptr;

namespace openmsx {

auto_ptr<HardwareConfig> HardwareConfig::createMachineConfig(
	MSXMotherBoard& motherBoard, const string& machineName)
{
	auto_ptr<HardwareConfig> result(
		new HardwareConfig(motherBoard, machineName));
	result->load("machines");
	return result;
}

auto_ptr<HardwareConfig> HardwareConfig::createExtensionConfig(
	MSXMotherBoard& motherBoard, const string& extensionName)
{
	auto_ptr<HardwareConfig> result(
		new HardwareConfig(motherBoard, extensionName));
	result->load("extensions");
	result->setName(extensionName);
	return result;
}

auto_ptr<HardwareConfig> HardwareConfig::createRomConfig(
	MSXMotherBoard& motherBoard, const string& romfile,
	const string& slotname, const vector<string>& options)
{
	auto_ptr<HardwareConfig> result(
		new HardwareConfig(motherBoard, "rom"));
	string_ref sramfile = FileOperations::getFilename(romfile);
	auto_ptr<FileContext> context(new UserFileContext("roms/" + sramfile));

	vector<string> ipsfiles;
	string mapper;

	bool romTypeOptionFound = false;

	// parse options
	for (vector<string>::const_iterator it = options.begin();
	     it != options.end(); ++it) {
		const string& option = *it++;
		if (it == options.end()) {
			throw MSXException("Missing argument for option \"" +
			                   option + '\"');
		}
		if (option == "-ips") {
			if (!FileOperations::isRegularFile(context->resolve(*it))) {
				throw MSXException("Invalid IPS file: " + *it);
			}
			ipsfiles.push_back(*it);
		} else if (option == "-romtype") {
			if (!romTypeOptionFound) {
				mapper = *it;
				romTypeOptionFound = true;
			} else {
				throw MSXException("Only one -romtype option is allowed");
			}
		} else {
			throw MSXException("Invalid option \"" + option + '\"');
		}
	}

	string resolvedFilename = FileOperations::getAbsolutePath(
		context->resolve(romfile));
	if (!FileOperations::isRegularFile(resolvedFilename)) {
		throw MSXException("Invalid ROM file: " + resolvedFilename);
	}

	auto_ptr<XMLElement> extension(new XMLElement("extension"));
	auto_ptr<XMLElement> devices(new XMLElement("devices"));
	auto_ptr<XMLElement> primary(new XMLElement("primary"));
	primary->addAttribute("slot", slotname);
	auto_ptr<XMLElement> secondary(new XMLElement("secondary"));
	secondary->addAttribute("slot", slotname);
	auto_ptr<XMLElement> device(new XMLElement("ROM"));
	device->addAttribute("id", "MSXRom");
	auto_ptr<XMLElement> mem(new XMLElement("mem"));
	mem->addAttribute("base", "0x0000");
	mem->addAttribute("size", "0x10000");
	device->addChild(mem);
	auto_ptr<XMLElement> rom(new XMLElement("rom"));
	rom->addChild(auto_ptr<XMLElement>(
		new XMLElement("resolvedFilename", resolvedFilename)));
	rom->addChild(auto_ptr<XMLElement>(
		new XMLElement("filename", romfile)));
	if (!ipsfiles.empty()) {
		auto_ptr<XMLElement> patches(new XMLElement("patches"));
		for (vector<string>::const_iterator it = ipsfiles.begin();
		     it != ipsfiles.end(); ++it) {
			patches->addChild(auto_ptr<XMLElement>(
				new XMLElement("ips", *it)));
		}
		rom->addChild(patches);
	}
	device->addChild(rom);
	auto_ptr<XMLElement> sound(new XMLElement("sound"));
	sound->addChild(auto_ptr<XMLElement>(
		new XMLElement("volume", "9000")));
	device->addChild(sound);
	device->addChild(auto_ptr<XMLElement>(
		new XMLElement("mappertype",
		               mapper.empty() ? "auto" : mapper)));
	device->addChild(auto_ptr<XMLElement>(
		new XMLElement("sramname", sramfile + ".SRAM")));

	secondary->addChild(device);
	primary->addChild(secondary);
	devices->addChild(primary);
	extension->addChild(devices);

	result->setConfig(extension);
	result->setName(romfile);
	result->setFileContext(context);

	return result;
}

HardwareConfig::HardwareConfig(MSXMotherBoard& motherBoard_, const string& hwName_)
	: motherBoard(motherBoard_)
	, hwName(hwName_)
{
	for (int ps = 0; ps < 4; ++ps) {
		for (int ss = 0; ss < 4; ++ss) {
			externalSlots[ps][ss] = false;
		}
		externalPrimSlots[ps] = false;
		expandedSlots[ps] = false;
		allocatedPrimarySlots[ps] = false;
	}
	userName = motherBoard.getUserName(hwName);
}

HardwareConfig::~HardwareConfig()
{
	motherBoard.freeUserName(hwName, userName);
#ifndef NDEBUG
	try {
		testRemove();
	} catch (MSXException& e) {
		std::cerr << e.getMessage() << std::endl;
		UNREACHABLE;
	}
#endif
	for (Devices::reverse_iterator it = devices.rbegin();
	     it != devices.rend(); ++it) {
		motherBoard.removeDevice(**it);
		delete *it;
	}
	CartridgeSlotManager& slotManager = motherBoard.getSlotManager();
	for (int ps = 0; ps < 4; ++ps) {
		for (int ss = 0; ss < 4; ++ss) {
			if (externalSlots[ps][ss]) {
				slotManager.removeExternalSlot(ps, ss);
			}
		}
		if (externalPrimSlots[ps]) {
			slotManager.removeExternalSlot(ps);
		}
		if (expandedSlots[ps]) {
			motherBoard.getCPUInterface().unsetExpanded(ps);
		}
		if (allocatedPrimarySlots[ps]) {
			slotManager.freePrimarySlot(ps, *this);
		}
	}
}

void HardwareConfig::testRemove() const
{
	Devices alreadyRemoved;
	for (Devices::const_reverse_iterator it = devices.rbegin();
	     it != devices.rend(); ++it) {
		(*it)->testRemove(alreadyRemoved);
		alreadyRemoved.push_back(*it);
	}
	CartridgeSlotManager& slotManager = motherBoard.getSlotManager();
	for (int ps = 0; ps < 4; ++ps) {
		for (int ss = 0; ss < 4; ++ss) {
			if (externalSlots[ps][ss]) {
				slotManager.testRemoveExternalSlot(ps, ss, *this);
			}
		}
		if (externalPrimSlots[ps]) {
			slotManager.testRemoveExternalSlot(ps, *this);
		}
		if (expandedSlots[ps]) {
			motherBoard.getCPUInterface().testUnsetExpanded(
				ps, alreadyRemoved);
		}
	}
}

const FileContext& HardwareConfig::getFileContext() const
{
	return *context;
}
void HardwareConfig::setFileContext(std::auto_ptr<FileContext> context_)
{
	context = context_;
}

const XMLElement& HardwareConfig::getConfig() const
{
	return *config;
}

void HardwareConfig::setConfig(std::auto_ptr<XMLElement> newConfig)
{
	assert(!config.get());
	config = newConfig;
}

const XMLElement& HardwareConfig::getDevices() const
{
	return getConfig().getChild("devices");
}

std::auto_ptr<XMLElement> HardwareConfig::loadConfig(const string& filename)
{
	try {
		LocalFileReference fileRef(filename);
		return XMLLoader::load(fileRef.getFilename(), "msxconfig2.dtd");
	} catch (XMLException& e) {
		throw MSXException(
			"Loading of hardware configuration failed: " +
			e.getMessage());
	}
}

void HardwareConfig::load(string_ref path)
{
	string filename = SystemFileContext().resolve(
		FileOperations::join(path, hwName, "hardwareconfig.xml"));
	setConfig(loadConfig(filename));

	assert(!userName.empty());
	string_ref baseName = FileOperations::getBaseName(filename);
	setFileContext(std::auto_ptr<FileContext>(
		new ConfigFileContext(baseName, hwName, userName)));
}

void HardwareConfig::parseSlots()
{
	// TODO this code does parsing for both 'expanded' and 'external' slots
	//      once machine and extensions are parsed separately move parsing
	//      of 'expanded' to MSXCPUInterface
	//
	XMLElement::Children primarySlots;
	getDevices().getChildren("primary", primarySlots);
	for (XMLElement::Children::const_iterator it = primarySlots.begin();
	     it != primarySlots.end(); ++it) {
		const string& primSlot = (*it)->getAttribute("slot");
		int ps = CartridgeSlotManager::getSlotNum(primSlot);
		if ((*it)->getAttributeAsBool("external", false)) {
			if (ps < 0) {
				throw MSXException(
				    "Cannot mark unspecified primary slot '" +
				    primSlot + "' as external");
			}
			createExternalSlot(ps);
			continue;
		}
		XMLElement::Children secondarySlots;
		(*it)->getChildren("secondary", secondarySlots);
		for (XMLElement::Children::const_iterator it2 = secondarySlots.begin();
		     it2 != secondarySlots.end(); ++it2) {
			const string& secSlot = (*it2)->getAttribute("slot");
			int ss = CartridgeSlotManager::getSlotNum(secSlot);
			if (ss < 0) {
				if ((ss >= -128) && (0 <= ps) && (ps < 4) &&
				    motherBoard.getCPUInterface().isExpanded(ps)) {
					ss += 128;
				} else {
					continue;
				}
			}
			if (ps < 0) {
				ps = getFreePrimarySlot();
				XMLElement* mutableElem = const_cast<XMLElement*>(*it);
				mutableElem->setAttribute("slot", StringOp::toString(ps));
			}
			createExpandedSlot(ps);
			if ((*it2)->getAttributeAsBool("external", false)) {
				createExternalSlot(ps, ss);
			}
		}
	}
}

void HardwareConfig::createDevices()
{
	createDevices(getDevices(), NULL, NULL);
}

void HardwareConfig::createDevices(const XMLElement& elem,
	const XMLElement* primary, const XMLElement* secondary)
{
	const XMLElement::Children& children = elem.getChildren();
	for (XMLElement::Children::const_iterator it = children.begin();
	     it != children.end(); ++it) {
		const XMLElement& sub = **it;
		const string& name = sub.getName();
		if (name == "primary") {
			createDevices(sub, &sub, secondary);
		} else if (name == "secondary") {
			createDevices(sub, primary, &sub);
		} else {
			std::auto_ptr<MSXDevice> device(DeviceFactory::create(
				DeviceConfig(*this, sub, primary, secondary)));
			if (device.get()) {
				addDevice(device.release());
			} else {
				motherBoard.getMSXCliComm().printWarning(
					"Deprecated device: \"" +
					name + "\", please upgrade your "
					"hardware descriptions.");
			}
		}
	}
}

void HardwareConfig::createExternalSlot(int ps)
{
	motherBoard.getSlotManager().createExternalSlot(ps);
	assert(!externalPrimSlots[ps]);
	externalPrimSlots[ps] = true;
}

void HardwareConfig::createExternalSlot(int ps, int ss)
{
	motherBoard.getSlotManager().createExternalSlot(ps, ss);
	assert(!externalSlots[ps][ss]);
	externalSlots[ps][ss] = true;
}

void HardwareConfig::createExpandedSlot(int ps)
{
	if (!expandedSlots[ps]) {
		motherBoard.getCPUInterface().setExpanded(ps);
		expandedSlots[ps] = true;
	}
}

int HardwareConfig::getFreePrimarySlot()
{
	int ps;
	motherBoard.getSlotManager().allocatePrimarySlot(ps, *this);
	assert(!allocatedPrimarySlots[ps]);
	allocatedPrimarySlots[ps] = true;
	return ps;
}

void HardwareConfig::addDevice(MSXDevice* device)
{
	motherBoard.addDevice(*device);
	devices.push_back(device);
}

const string& HardwareConfig::getName() const
{
	return name;
}

void HardwareConfig::setName(const string& proposedName)
{
	if (!motherBoard.findExtension(proposedName)) {
		name = proposedName;
	} else {
		unsigned n = 0;
		do {
			name = StringOp::Builder() << proposedName << " (" << ++n << ')';
		} while (motherBoard.findExtension(name));
	}
}


// version 1: initial version
// version 2: moved FileContext here (was part of config)
template<typename Archive>
void HardwareConfig::serialize(Archive& ar, unsigned version)
{
	// filled-in by constructor:
	//   motherBoard, hwName, userName
	// filled-in by parseSlots()
	//   externalSlots, externalPrimSlots, expandedSlots, allocatedPrimarySlots

	if (ar.versionBelow(version, 2)) {
		XMLElement::getLastSerializedFileContext(); // clear any previous value
	}
	ar.serialize("config", config); // fills in getLastSerializedFileContext()
	if (ar.versionAtLeast(version, 2)) {
		ar.serialize("context", context);
	} else {
		context = XMLElement::getLastSerializedFileContext();
		assert(context.get());
	}
	if (ar.isLoader()) {
		if (!motherBoard.getMachineConfig()) {
			// must be done before parseSlots()
			motherBoard.setMachineConfig(this);
		} else {
			// already set because this is an extension
		}
		parseSlots();
		createDevices();
	}
	// only (polymorphically) initialize devices, they are already created
	for (Devices::const_iterator it = devices.begin();
	     it != devices.end(); ++it) {
		ar.serializePolymorphic("device", **it);
	}
	ar.serialize("name", name);
}
INSTANTIATE_SERIALIZE_METHODS(HardwareConfig);

} // namespace openmsx
