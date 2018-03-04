// $Id: EmuTime.cc 8352 2008-11-12 18:39:08Z m9710797 $

#include "EmuTime.hh"
#include "serialize.hh"
#include <limits>
#include <iostream>

namespace openmsx {

const EmuTime EmuTime::zero(uint64(0));
const EmuTime EmuTime::infinity(std::numeric_limits<uint64>::max());

std::ostream& operator<<(std::ostream& os, EmuTime::param et)
{
	os << et.time;
	return os;
}

template<typename Archive>
void EmuTime::serialize(Archive& ar, unsigned /*version*/)
{
	ar.serialize("time", time);
}
INSTANTIATE_SERIALIZE_METHODS(EmuTime);

} // namespace openmsx
