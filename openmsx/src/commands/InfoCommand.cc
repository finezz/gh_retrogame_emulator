// $Id: InfoCommand.cc 12805 2012-08-13 19:26:18Z m9710797 $

#include "InfoCommand.hh"
#include "InfoTopic.hh"
#include "TclObject.hh"
#include "MSXCommandController.hh"
#include "CliComm.hh"
#include "CommandException.hh"
#include "MSXMotherBoard.hh"
#include "unreachable.hh"
#include <iostream>
#include <cassert>

using std::set;
using std::string;
using std::vector;

namespace openmsx {

InfoCommand::InfoCommand(CommandController& commandController, const string& name)
	: Command(commandController, name)
{
}

InfoCommand::~InfoCommand()
{
	assert(infoTopics.empty());
}

void InfoCommand::registerTopic(InfoTopic& topic, string_ref name)
{
#ifndef NDEBUG
	if (infoTopics.find(name) != infoTopics.end()) {
		std::cerr << "INTERNAL ERROR: already have a info topic with "
		             "name " << name << std::endl;
		UNREACHABLE;
	}
#endif
	infoTopics[name] = &topic;
}

void InfoCommand::unregisterTopic(InfoTopic& topic, string_ref name)
{
	(void)topic;
	if (infoTopics.find(name) == infoTopics.end()) {
		std::cerr << "INTERNAL ERROR: can't unregister topic with name "
			"name " << name << ", not found!" << std::endl;
		UNREACHABLE;
	}
	assert(infoTopics[name] == &topic);
	infoTopics.erase(name);
}

// Command

void InfoCommand::execute(const vector<TclObject>& tokens,
                          TclObject& result)
{
	switch (tokens.size()) {
	case 1:
		// list topics
		for (InfoTopics::const_iterator it = infoTopics.begin();
		     it != infoTopics.end(); ++it) {
			result.addListElement(it->first());
		}
		break;
	default:
		// show info about topic
		assert(tokens.size() >= 2);
		string_ref topic = tokens[1].getString();
		InfoTopics::const_iterator it = infoTopics.find(topic);
		if (it == infoTopics.end()) {
			throw CommandException("No info on: " + topic);
		}
		it->second->execute(tokens, result);
		break;
	}
}

string InfoCommand::help(const vector<string>& tokens) const
{
	string result;
	switch (tokens.size()) {
	case 1:
		// show help on info cmd
		result = "Show info on a certain topic\n"
		         " info [topic] [...]\n";
		break;
	default:
		// show help on a certain topic
		assert(tokens.size() >= 2);
		InfoTopics::const_iterator it = infoTopics.find(tokens[1]);
		if (it == infoTopics.end()) {
			throw CommandException("No info on: " + tokens[1]);
		}
		result = it->second->help(tokens);
		break;
	}
	return result;
}

void InfoCommand::tabCompletion(vector<string>& tokens) const
{
	switch (tokens.size()) {
	case 2: {
		// complete topic
		set<string> topics;
		for (InfoTopics::const_iterator it = infoTopics.begin();
		     it != infoTopics.end(); ++it) {
			topics.insert(it->first().str());
		}
		completeString(tokens, topics);
		break;
	}
	default:
		// show help on a certain topic
		assert(tokens.size() >= 3);
		InfoTopics::const_iterator it = infoTopics.find(tokens[1]);
		if (it != infoTopics.end()) {
			it->second->tabCompletion(tokens);
		}
		break;
	}
}

} // namespace openmsx
