// $Id: CheckedRam.hh 12740 2012-07-19 18:18:29Z m9710797 $

#ifndef CHECKEDRAM_HH
#define CHECKEDRAM_HH

#include "CacheLine.hh"
#include "Observer.hh"
#include "openmsx.hh"
#include "noncopyable.hh"
#include <vector>
#include <bitset>
#include <memory>

namespace openmsx {

class DeviceConfig;
class Ram;
class MSXCPU;
class Setting;
class TclCallback;

/**
 * This class keeps track of which bytes in the Ram have been written to. It
 * can be used for debugging MSX programs, because you can see if you are
 * trying to read/execute uninitialized memory. Currently all normal RAM
 * (MSXRam) and all normal memory mappers (MSXMemoryMappers) use CheckedRam. On
 * the turboR, only the normal memory mapper runs via CheckedRam. The RAM
 * accessed in DRAM mode or via the ROM mapper are unchecked! Note that there
 * is basically no overhead for using CheckedRam over Ram, thanks to Wouter.
 */
class CheckedRam : private Observer<Setting>, private noncopyable
{
public:
	CheckedRam(const DeviceConfig& config, const std::string& name,
	           const std::string& description, unsigned size);
	virtual ~CheckedRam();

	byte read(unsigned addr);
	byte peek(unsigned addr) const;
	void write(unsigned addr, const byte value);

	const byte* getReadCacheLine(unsigned addr) const;
	byte* getWriteCacheLine(unsigned addr) const;

	unsigned getSize() const;
	void clear();

	/**
	 * Give access to the unchecked Ram. No problem to use it, but there
	 * will just be no checking done! Keep in mind that you should use this
	 * consistently, so that the initialized-administration will be always
	 * up to date!
	 */
	Ram& getUncheckedRam() const;

	// TODO
	//template<typename Archive>
	//void serialize(Archive& ar, unsigned version);

private:
	void init();

	// Observer<Setting>
	virtual void update(const Setting& setting);

	std::vector<bool> completely_initialized_cacheline;
	std::vector<std::bitset<CacheLine::SIZE> > uninitialized;
	const std::auto_ptr<Ram> ram;
	MSXCPU& msxcpu;
	std::auto_ptr<TclCallback> umrCallback;
};

} // namespace openmsx

#endif
