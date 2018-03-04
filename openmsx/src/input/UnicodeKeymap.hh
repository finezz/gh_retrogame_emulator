// $Id: UnicodeKeymap.hh 12622 2012-06-14 20:11:33Z m9710797 $

#ifndef UNICODEKEYMAP_HH
#define UNICODEKEYMAP_HH

#include "openmsx.hh"
#include "string_ref.hh"
#include <map>
#include <cassert>

namespace openmsx {

class UnicodeKeymap
{
public:
	struct KeyInfo {
		KeyInfo(byte row_, byte keymask_, byte modmask_)
			: row(row_), keymask(keymask_), modmask(modmask_)
		{
			if (keymask == 0) {
				assert(row     == 0);
				assert(modmask == 0);
			}
		}
		KeyInfo()
			: row(0), keymask(0), modmask(0)
		{
		}
		byte row, keymask, modmask;
	};

	explicit UnicodeKeymap(string_ref keyboardType);

	KeyInfo get(int unicode) const;
	KeyInfo getDeadkey(unsigned n) const;

private:
	static const unsigned NUM_DEAD_KEYS = 3;

	void parseUnicodeKeymapfile(const char* begin, const char* end);

	typedef std::map<int, KeyInfo> Mapdata;
	Mapdata mapdata;
	KeyInfo deadKeys[NUM_DEAD_KEYS];
	const KeyInfo emptyInfo;
};

} // namespace openmsx

#endif
