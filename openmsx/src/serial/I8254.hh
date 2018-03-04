// $Id: I8254.hh 8352 2008-11-12 18:39:08Z m9710797 $

// This class implements the Intel 8254 chip (and 8253)
//
// * Only the 8254 is emulated, no surrounding hardware.
//   Use the class I8254Interface to do that.

#ifndef I8254_HH
#define I8254_HH

#include "EmuTime.hh"
#include "openmsx.hh"
#include "noncopyable.hh"
#include <memory>

namespace openmsx {

class Scheduler;
class Counter;
class ClockPin;
class ClockPinListener;

class I8254 : private noncopyable
{
public:
	I8254(Scheduler& scheduler, ClockPinListener* output0,
	      ClockPinListener* output1, ClockPinListener* output2,
	      EmuTime::param time);
	~I8254();

	void reset(EmuTime::param time);
	byte readIO(word port, EmuTime::param time);
	byte peekIO(word port, EmuTime::param time) const;
	void writeIO(word port, byte value, EmuTime::param time);

	void setGate(unsigned counter, bool status, EmuTime::param time);
	ClockPin& getClockPin(unsigned cntr);
	ClockPin& getOutputPin(unsigned cntr);

	template<typename Archive>
	void serialize(Archive& ar, unsigned version);

private:
	void readBackHelper(byte value, unsigned cntr, EmuTime::param time);

	std::auto_ptr<Counter> counter[3];
};

} // namespace openmsx

#endif
