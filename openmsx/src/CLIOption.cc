// $Id: CLIOption.cc 11609 2010-07-22 21:46:22Z m9710797 $

#include "CLIOption.hh"
#include "MSXException.hh"

using std::string;
using std::deque;

namespace openmsx {

// class CLIOption

string CLIOption::getArgument(const string& option, deque<string>& cmdLine) const
{
	if (cmdLine.empty()) {
		throw FatalError("Missing argument for option \"" + option + '\"');
	}
	string argument = cmdLine.front();
	cmdLine.pop_front();
	return argument;
}

string CLIOption::peekArgument(const deque<string>& cmdLine) const
{
	return cmdLine.empty() ? "" : cmdLine.front();
}

} // namespace openmsx
