// $Id: BootBlocks.hh 5733 2006-10-02 13:16:39Z m9710797 $

#ifndef BOOTBLOCKS_HH
#define BOOTBLOCKS_HH

#include "openmsx.hh"

namespace openmsx {

class BootBlocks
{
public:
	// bootblock created with regular nms8250 and '_format'
	static const byte dos1BootBlock[512];

	// bootblock created with nms8250 and MSX-DOS 2.20
	static const byte dos2BootBlock[512];
};

} // namespace openmsx

#endif
