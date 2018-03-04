// $Id: GlobalCliComm.cc 12625 2012-06-14 20:13:15Z m9710797 $

#include "GlobalCliComm.hh"
#include "CliListener.hh"
#include "Thread.hh"
#include "ScopedAssign.hh"
#include <algorithm>
#include <cassert>
#include <iostream>

namespace openmsx {

GlobalCliComm::GlobalCliComm()
	: sem(1)
	, delivering(false)
{
}

GlobalCliComm::~GlobalCliComm()
{
	assert(Thread::isMainThread());
	assert(!delivering);

	ScopedLock lock(sem);
	for (Listeners::const_iterator it = listeners.begin();
	     it != listeners.end(); ++it) {
		delete *it;
	}
}

void GlobalCliComm::addListener(CliListener* listener)
{
	// can be called from any thread
	ScopedLock lock(sem);
	listeners.push_back(listener);
}

void GlobalCliComm::removeListener(CliListener* listener)
{
	// can be called from any thread
	ScopedLock lock(sem);
	Listeners::iterator it = find(listeners.begin(), listeners.end(), listener);
	assert(it != listeners.end());
	listeners.erase(it);
}

void GlobalCliComm::log(LogLevel level, string_ref message)
{
	assert(Thread::isMainThread());

	if (delivering) {
		// Don't allow recursive calls, this would hang while trying to
		// acquire the Semaphore below. But also when we would change
		// this to a recursive-mutex, this could result in an infinite
		// loop.
		// One example of a recursive invocation is when something goes
		// wrong in the Tcl proc attached to message_callback (e.g. the
		// font used to display the message could not be loaded).
		std::cerr << "Recursive cliComm message: " << message << std::endl;
		return;
	}
	ScopedAssign<bool> sa(delivering, true);

	ScopedLock lock(sem);
	if (!listeners.empty()) {
		for (Listeners::const_iterator it = listeners.begin();
		     it != listeners.end(); ++it) {
			(*it)->log(level, message);
		}
	} else {
		// don't let the message get lost
		std::cerr << message << std::endl;
	}
}

void GlobalCliComm::update(UpdateType type, string_ref name, string_ref value)
{
	assert(type < NUM_UPDATES);
	PrevValue::iterator it = prevValues[type].find(name);
	if (it != prevValues[type].end()) {
		if (it->second == value) {
			return;
		}
		it->second.assign(value.data(), value.size());
	} else {
		prevValues[type][name].assign(value.data(), value.size());
	}
	updateHelper(type, "", name, value);
}

void GlobalCliComm::updateHelper(UpdateType type, string_ref machine,
                                 string_ref name, string_ref value)
{
	assert(Thread::isMainThread());
	ScopedLock lock(sem);
	for (Listeners::const_iterator it = listeners.begin();
	     it != listeners.end(); ++it) {
		(*it)->update(type, machine, name, value);
	}
}

} // namespace openmsx
