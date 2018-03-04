// $Id: Rom.hh 12587 2012-06-06 20:31:32Z m9710797 $

#ifndef ROM_HH
#define ROM_HH

#include "MemBuffer.hh"
#include "sha1.hh"
#include "openmsx.hh"
#include "noncopyable.hh"
#include <string>
#include <memory>
#include <cassert>

namespace openmsx {

class MSXMotherBoard;
class XMLElement;
class DeviceConfig;
class File;
class FileContext;
class CliComm;
class RomDebuggable;

class Rom : private noncopyable
{
public:
	Rom(const std::string& name, const std::string& description,
	    const DeviceConfig& config, const std::string& id = "");
	virtual ~Rom();

	const byte& operator[](unsigned address) const {
		assert(address < size);
		return rom[address];
	}
	unsigned getSize() const { return size; }

	std::string getFilename() const;
	const std::string& getName() const;
	const std::string& getDescription() const;
	const Sha1Sum& getOriginalSHA1() const;
	const Sha1Sum& getPatchedSHA1() const;

private:
	void init(MSXMotherBoard& motherBoard, const XMLElement& config,
	          const FileContext& context);
	bool checkSHA1(const XMLElement& config);

	const byte* rom;
	MemBuffer<byte> extendedRom;

	std::auto_ptr<File> file;

	mutable Sha1Sum originalSha1;
	Sha1Sum patchedSha1;
	std::string name;
	const std::string description;
	unsigned size;

	// This must come after 'name':
	//   the destructor of RomDebuggable calls Rom::getName(), which still
	//   needs the Rom::name member.
	std::auto_ptr<RomDebuggable> romDebuggable;
};

} // namespace openmsx

#endif
