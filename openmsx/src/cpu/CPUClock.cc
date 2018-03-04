// $Id: CPUClock.cc 12813 2012-08-13 20:04:55Z m9710797 $

#include "CPUClock.hh"
#include "Scheduler.hh"
#include "serialize.hh"

namespace openmsx {

CPUClock::CPUClock(EmuTime::param time, Scheduler& scheduler_)
	: clock(time)
	, scheduler(scheduler_)
	, remaining(-1), limit(-1), limitEnabled(false)
{
}

void CPUClock::setLimit(EmuTime::param time)
{
	if (limitEnabled) {
		sync();
		assert(remaining == limit);
		int newLimit = std::min(15000u, clock.getTicksTillUp(time) - 1);
		if (limit < 0) {
			limit = newLimit;
		} else {
			limit = std::min(limit, newLimit);
		}
		remaining = limit;
	} else {
		assert(limit < 0);
	}
}

void CPUClock::enableLimit(bool enable_)
{
	limitEnabled = enable_;
	if (limitEnabled) {
		setLimit(scheduler.getNext());
	} else {
		int extra = limit - remaining;
		limit = -1;
		remaining = limit - extra;
	}
}

void CPUClock::advanceTime(EmuTime::param time)
{
	remaining = limit;
	clock.advance(time);
	setLimit(scheduler.getNext());
}

template<typename Archive>
void CPUClock::serialize(Archive& ar, unsigned /*version*/)
{
	sync();
	ar.serialize("clock", clock);
}
INSTANTIATE_SERIALIZE_METHODS(CPUClock);

} // namespace openmsx
