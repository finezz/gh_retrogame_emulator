// $Id: DummyAY8910Periphery.hh 12305 2011-10-26 07:27:49Z bifimsx $

#ifndef DUMMYAY8910PERIPHERY_HH
#define DUMMYAY8910PERIPHERY_HH

#include "AY8910Periphery.hh"

namespace openmsx {

class DummyAY8910Periphery : public AY8910Periphery
{
public:
	static DummyAY8910Periphery& instance()
	{
		static DummyAY8910Periphery oneInstance;
		return oneInstance;
	}

	virtual byte readA(EmuTime::param /*time*/) { return 255; }
	virtual byte readB(EmuTime::param /*time*/) { return 255; }
	virtual void writeA(byte /*value*/, EmuTime::param /*time*/) {}
	virtual void writeB(byte /*value*/, EmuTime::param /*time*/) {}

private:
	DummyAY8910Periphery() {}
	virtual ~DummyAY8910Periphery() {}
};

}; // namespace openmsx

#endif
