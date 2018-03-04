// $Id: RecordedCommand.cc 12805 2012-08-13 19:26:18Z m9710797 $

#include "RecordedCommand.hh"
#include "CommandController.hh"
#include "StateChangeDistributor.hh"
#include "TclObject.hh"
#include "Scheduler.hh"
#include "StateChange.hh"
#include "ScopedAssign.hh"
#include "serialize.hh"
#include "serialize_stl.hh"
#include "checked_cast.hh"
#include "unreachable.hh"

using std::vector;
using std::string;

namespace openmsx {


RecordedCommand::RecordedCommand(CommandController& commandController,
                                 StateChangeDistributor& stateChangeDistributor_,
                                 Scheduler& scheduler_,
                                 string_ref name)
	: Command(commandController, name)
	, stateChangeDistributor(stateChangeDistributor_)
	, scheduler(scheduler_)
	, dummyResultObject(new TclObject(getInterpreter()))
	, currentResultObject(dummyResultObject.get())
{
	stateChangeDistributor.registerListener(*this);
}

RecordedCommand::~RecordedCommand()
{
	stateChangeDistributor.unregisterListener(*this);
}

void RecordedCommand::execute(const vector<TclObject>& tokens,
                              TclObject& result)
{
	EmuTime::param time = scheduler.getCurrentTime();
	if (needRecord(tokens)) {
		ScopedAssign<TclObject*> sa(currentResultObject, &result);
		stateChangeDistributor.distributeNew(
			StateChangeDistributor::EventPtr(
				new MSXCommandEvent(tokens, time)));
	} else {
		execute(tokens, result, time);
	}
}

bool RecordedCommand::needRecord(const vector<TclObject>& tokens) const
{
	vector<string> strings;
	strings.reserve(tokens.size());
	for (vector<TclObject>::const_iterator it = tokens.begin();
	     it != tokens.end(); ++it) {
		strings.push_back(it->getString().str());
	}
	return needRecord(strings);
}

bool RecordedCommand::needRecord(const vector<string>& /*tokens*/) const
{
	return true;
}

static string_ref getBaseName(string_ref str)
{
	string_ref::size_type pos = str.rfind("::");
	return (pos == string_ref::npos) ? str : str.substr(pos + 2);
}

void RecordedCommand::signalStateChange(const shared_ptr<StateChange>& event)
{
	MSXCommandEvent* commandEvent =
		dynamic_cast<MSXCommandEvent*>(event.get());
	if (!commandEvent) return;

	const vector<TclObject>& tokens = commandEvent->getTokens();
	if (getBaseName(tokens[0].getString()) != getName()) return;

	if (needRecord(tokens)) {
		execute(tokens, *currentResultObject, commandEvent->getTime());
	} else {
		// Normally this shouldn't happen. But it's possible in case
		// we're replaying a replay file that has manual edits in the
		// event log. It's crucial for security that we don't blindly
		// execute such commands. We already only execute
		// RecordedCommands, but we also need a strict check that
		// only commands that would be recorded are also replayed.
		// For example:
		//   debug set_bp 0x0038 true {<some-arbitrary-Tcl-command>}
		// The debug write/write_block commands should be recorded and
		// replayed, but via the set_bp it would be possible to
		// execute arbitrary Tcl code.
	}
}

void RecordedCommand::stopReplay(EmuTime::param /*time*/)
{
	// nothing
}

void RecordedCommand::execute(const vector<TclObject>& tokens,
                              TclObject& result, EmuTime::param time)
{
	vector<string> strings;
	strings.reserve(tokens.size());
	for (vector<TclObject>::const_iterator it = tokens.begin();
	     it != tokens.end(); ++it) {
		strings.push_back(it->getString().str());
	}
	result.setString(execute(strings, time));
}

string RecordedCommand::execute(const vector<string>& /*tokens*/,
                                EmuTime::param /*time*/)
{
	// either this method or the method above should be reimplemented
	// by the subclasses
	UNREACHABLE; return "";
}


// class MSXCommandEvent

MSXCommandEvent::MSXCommandEvent(const vector<string>& tokens_, EmuTime::param time)
	: StateChange(time)
{
	for (vector<string>::const_iterator it = tokens_.begin();
	     it != tokens_.end(); ++it) {
		tokens.push_back(TclObject(*it));
	}
}

MSXCommandEvent::MSXCommandEvent(const vector<TclObject>& tokens_, EmuTime::param time)
	: StateChange(time)
	, tokens(tokens_)
{
}

MSXCommandEvent::~MSXCommandEvent()
{
}

const vector<TclObject>& MSXCommandEvent::getTokens() const
{
	return tokens;
}

template<typename Archive>
void MSXCommandEvent::serialize(Archive& ar, unsigned /*version*/)
{
	ar.template serializeBase<StateChange>(*this);

	// serialize vector<TclObject> as vector<string>
	vector<string> str;
	if (!ar.isLoader()) {
		for (vector<TclObject>::const_iterator it = tokens.begin();
		     it != tokens.end(); ++it) {
			str.push_back(it->getString().str());
		}
	}
	ar.serialize("tokens", str);
	if (ar.isLoader()) {
		assert(tokens.empty());
		for (vector<string>::const_iterator it = str.begin();
		     it != str.end(); ++it) {
			tokens.push_back(TclObject(*it));
		}
	}
}
REGISTER_POLYMORPHIC_CLASS(StateChange, MSXCommandEvent, "MSXCommandEvent");

} // namespace openmsx
