// $Id: EmuTimer.hh 12255 2011-08-30 19:35:26Z m9710797 $

#ifndef EMUTIMER_HH
#define EMUTIMER_HH

#include "Schedulable.hh"
#include "DynamicClock.hh"
#include "openmsx.hh"
#include <memory>

namespace openmsx {

class EmuTimerCallback
{
public:
	virtual void callback(byte value) = 0;
protected:
	virtual ~EmuTimerCallback() {}
};


class EmuTimer : public Schedulable
{
public:
	static std::auto_ptr<EmuTimer> createOPM_1(
		Scheduler& scheduler, EmuTimerCallback& cb);
	static std::auto_ptr<EmuTimer> createOPM_2(
		Scheduler& scheduler, EmuTimerCallback& cb);
	static std::auto_ptr<EmuTimer> createOPL3_1(
		Scheduler& scheduler, EmuTimerCallback& cb);
	static std::auto_ptr<EmuTimer> createOPL3_2(
		Scheduler& scheduler, EmuTimerCallback& cb);
	static std::auto_ptr<EmuTimer> createOPL4_1(
		Scheduler& scheduler, EmuTimerCallback& cb);
	static std::auto_ptr<EmuTimer> createOPL4_2(
		Scheduler& scheduler, EmuTimerCallback& cb);

	void setValue(int value);
	void setStart(bool start, EmuTime::param time);

	template<typename Archive>
	void serialize(Archive& ar, unsigned version);

private:
	EmuTimer(Scheduler& scheduler, EmuTimerCallback& cb,
	         byte flag, unsigned freq_num, unsigned freq_denom,
	         unsigned maxval);
	virtual void executeUntil(EmuTime::param time, int userData);
	void schedule(EmuTime::param time);
	void unschedule();

	EmuTimerCallback& cb;
	DynamicClock clock;
	const unsigned maxval;
	int count;
	const byte flag;
	bool counting;
};

} // namespace openmsx

#endif
