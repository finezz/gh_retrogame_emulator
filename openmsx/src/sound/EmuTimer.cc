// $Id: EmuTimer.cc 12255 2011-08-30 19:35:26Z m9710797 $

#include "EmuTimer.hh"
#include "serialize.hh"

namespace openmsx {

std::auto_ptr<EmuTimer> EmuTimer::createOPM_1(
	Scheduler& scheduler, EmuTimerCallback& cb)
{
	return std::auto_ptr<EmuTimer>(new EmuTimer(
		scheduler, cb, 0x40,  3579545, 64 * 2     , 1024));
}

std::auto_ptr<EmuTimer> EmuTimer::createOPM_2(
	Scheduler& scheduler, EmuTimerCallback& cb)
{
	return std::auto_ptr<EmuTimer>(new EmuTimer(
		scheduler, cb, 0x20,  3579545, 64 * 2 * 16, 256));
}

std::auto_ptr<EmuTimer> EmuTimer::createOPL3_1(
	Scheduler& scheduler, EmuTimerCallback& cb)
{
	return std::auto_ptr<EmuTimer>(new EmuTimer(
		scheduler, cb, 0x40,  3579545, 72 *  4    , 256));
}

std::auto_ptr<EmuTimer> EmuTimer::createOPL3_2(
	Scheduler& scheduler, EmuTimerCallback& cb)
{
	return std::auto_ptr<EmuTimer>(new EmuTimer(
		scheduler, cb, 0x20,  3579545, 72 *  4 * 4, 256));
}

std::auto_ptr<EmuTimer> EmuTimer::createOPL4_1(
	Scheduler& scheduler, EmuTimerCallback& cb)
{
	return std::auto_ptr<EmuTimer>(new EmuTimer(
		scheduler, cb, 0x40, 33868800, 72 * 38    , 256));
}

std::auto_ptr<EmuTimer> EmuTimer::createOPL4_2(
	Scheduler& scheduler, EmuTimerCallback& cb)
{
	return std::auto_ptr<EmuTimer>(new EmuTimer(
		scheduler, cb, 0x20, 33868800, 72 * 38 * 4, 256));
}


EmuTimer::EmuTimer(Scheduler& scheduler, EmuTimerCallback& cb_,
                   byte flag_, unsigned freq_num, unsigned freq_denom,
                   unsigned maxval_)
	: Schedulable(scheduler), cb(cb_)
	, clock(EmuTime::dummy())
	, maxval(maxval_), count(maxval_)
	, flag(flag_), counting(false)
{
	clock.setFreq(freq_num, freq_denom);
}

void EmuTimer::setValue(int value)
{
	count = maxval - value;
}

void EmuTimer::setStart(bool start, EmuTime::param time)
{
	if (start != counting) {
		counting = start;
		if (start) {
			schedule(time);
		} else {
			unschedule();
		}
	}
}

void EmuTimer::schedule(EmuTime::param time)
{
	clock.reset(time);
	clock += count;
	setSyncPoint(clock.getTime());
}

void EmuTimer::unschedule()
{
	removeSyncPoint();
}

void EmuTimer::executeUntil(EmuTime::param time, int /*userData*/)
{
	cb.callback(flag);
	schedule(time);
}

template<typename Archive>
void EmuTimer::serialize(Archive& ar, unsigned /*version*/)
{
	ar.template serializeBase<Schedulable>(*this);
	ar.serialize("count", count);
	ar.serialize("counting", counting);
}
INSTANTIATE_SERIALIZE_METHODS(EmuTimer);

} // namespace openmsx
