// $Id: ResampleTrivial.cc 12241 2011-08-27 14:58:39Z m9710797 $

#include "ResampleTrivial.hh"
#include "ResampledSoundDevice.hh"
#include "build-info.hh"
#include <cassert>

namespace openmsx {

ResampleTrivial::ResampleTrivial(ResampledSoundDevice& input_)
	: input(input_)
{
}

bool ResampleTrivial::generateOutput(int* dataOut, unsigned num,
                                     EmuTime::param /*time*/)
{
#if ASM_X86
	assert((long(dataOut) & 15) == 0); // must be 16-byte aligned
#endif
	return input.generateInput(dataOut, num);
}

} // namespace openmsx
