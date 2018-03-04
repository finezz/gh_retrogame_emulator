// $Id: Probe.cc 12018 2011-03-13 10:05:58Z mthuurne $

#include "Probe.hh"
#include "Debugger.hh"

namespace openmsx {

ProbeBase::ProbeBase(Debugger& debugger_, const std::string& name_,
                     const std::string& description_)
	: debugger(debugger_)
	, name(name_)
	, description(description_)
{
	debugger.registerProbe(name, *this);
}

ProbeBase::~ProbeBase()
{
	debugger.unregisterProbe(name, *this);
}

const std::string& ProbeBase::getName() const
{
	return name;
}

const std::string& ProbeBase::getDescription() const
{
	return description;
}


Probe<void>::Probe(Debugger& debugger, const std::string& name,
                   const std::string& description)
	: ProbeBase(debugger, name, description)
{
}

void Probe<void>::signal()
{
	notify();
}

std::string Probe<void>::getValue() const
{
	return "";
}

} // namespace openmsx
