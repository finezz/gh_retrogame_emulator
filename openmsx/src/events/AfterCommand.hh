// $Id: AfterCommand.hh 12809 2012-08-13 19:27:49Z m9710797 $

#ifndef AFTERCOMMAND_HH
#define AFTERCOMMAND_HH

#include "Command.hh"
#include "EventListener.hh"
#include "Event.hh"
#include "shared_ptr.hh"
#include <vector>

namespace openmsx {

class Reactor;
class EventDistributor;
class EventDistributor;
class CommandController;
class AfterCmd;
class Event;

class AfterCommand : public Command, private EventListener
{
public:
	typedef shared_ptr<const Event> EventPtr;

	AfterCommand(Reactor& reactor,
	             EventDistributor& eventDistributor,
	             CommandController& commandController);
	virtual ~AfterCommand();

	virtual void execute(const std::vector<TclObject>& tokens,
	                     TclObject& result);
	virtual std::string help(const std::vector<std::string>& tokens) const;
	virtual void tabCompletion(std::vector<std::string>& tokens) const;

private:
	template<typename PRED> void executeMatches(PRED pred);
	template<EventType T> void executeEvents();
	template<EventType T> void afterEvent(
	                   const std::vector<TclObject>& tokens, TclObject& result);
	void afterInputEvent(const EventPtr& event,
	                   const std::vector<TclObject>& tokens, TclObject& result);
	void afterTclTime (int ms,
	                   const std::vector<TclObject>& tokens, TclObject& result);
	void afterTime    (const std::vector<TclObject>& tokens, TclObject& result);
	void afterRealTime(const std::vector<TclObject>& tokens, TclObject& result);
	void afterIdle    (const std::vector<TclObject>& tokens, TclObject& result);
	void afterInfo    (const std::vector<TclObject>& tokens, TclObject& result);
	void afterCancel  (const std::vector<TclObject>& tokens, TclObject& result);
	void executeRealTime();

	// EventListener
	virtual int signalEvent(const shared_ptr<const Event>& event);

	typedef std::vector<shared_ptr<AfterCmd> > AfterCmds;
	AfterCmds afterCmds;
	Reactor& reactor;
	EventDistributor& eventDistributor;

	friend class AfterCmd;
};

} // namespace openmsx

#endif
