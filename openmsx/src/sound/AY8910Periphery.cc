// $Id: AY8910Periphery.cc 8352 2008-11-12 18:39:08Z m9710797 $

#include "AY8910Periphery.hh"

namespace openmsx {

AY8910Periphery::AY8910Periphery()
{
}

AY8910Periphery::~AY8910Periphery()
{
}

byte AY8910Periphery::readA(EmuTime::param /*time*/)
{
	return 0xFF; // unused bits are 1
}

byte AY8910Periphery::readB(EmuTime::param /*time*/)
{
	return 0xFF; // unused bits are 1
}

void AY8910Periphery::writeA(byte /*value*/, EmuTime::param /*time*/)
{
	// nothing connected
}

void AY8910Periphery::writeB(byte /*value*/, EmuTime::param /*time*/)
{
	// nothing connected
}

} // namespace openmsx
