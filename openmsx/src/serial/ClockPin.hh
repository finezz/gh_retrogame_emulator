// $Id: ClockPin.hh 11722 2010-10-23 11:24:57Z m9710797 $

#ifndef CLOCKPIN_HH
#define CLOCKPIN_HH

#include "EmuTime.hh"
#include "Schedulable.hh"

namespace openmsx {

class Scheduler;
class ClockPin;

class ClockPinListener
{
public:
	virtual void signal(ClockPin& pin, EmuTime::param time) = 0;
	virtual void signalPosEdge(ClockPin& pin, EmuTime::param time) = 0;
protected:
	virtual ~ClockPinListener() {}
};

class ClockPin : public Schedulable
{
public:
	explicit ClockPin(Scheduler& scheduler, ClockPinListener* listener = NULL);

	// input side
	void setState(bool status, EmuTime::param time);
	void setPeriodicState(EmuDuration::param total,
	                      EmuDuration::param hi, EmuTime::param time);

	// output side
	bool getState(EmuTime::param time) const;
	bool isPeriodic() const;
	EmuDuration::param getTotalDuration() const;
	EmuDuration::param getHighDuration() const;
	int getTicksBetween(EmuTime::param begin,
	                    EmuTime::param end) const;

	// control
	void generateEdgeSignals(bool wanted, EmuTime::param time);

	template<typename Archive>
	void serialize(Archive& ar, unsigned version);

private:
	void unschedule();
	void schedule(EmuTime::param time);
	virtual void executeUntil(EmuTime::param time, int userData);

	ClockPinListener* const listener;

	EmuDuration totalDur;
	EmuDuration hiDur;
	EmuTime referenceTime;

	bool periodic;
	bool status;
	bool signalEdge;
};

} // namespace openmsx

#endif
