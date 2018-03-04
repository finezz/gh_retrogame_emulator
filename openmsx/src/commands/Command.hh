// $Id: Command.hh 12805 2012-08-13 19:26:18Z m9710797 $

#ifndef COMMAND_HH
#define COMMAND_HH

#include "Completer.hh"
#include "string_ref.hh"
#include <vector>

namespace openmsx {

class CommandController;
class GlobalCommandController;
class Interpreter;
class TclObject;
class CliComm;

class CommandCompleter : public Completer
{
public:
	CommandController& getCommandController() const;

protected:
	CommandCompleter(CommandController& commandController,
	                 string_ref name);
	virtual ~CommandCompleter();

	GlobalCommandController& getGlobalCommandController() const;
	Interpreter& getInterpreter() const;
	CliComm& getCliComm() const;

private:
	CommandController& commandController;
};


class Command : public CommandCompleter
{
public:
	/** Execute this command.
	  * @param tokens Tokenized command line;
	  *     tokens[0] is the command itself.
	  * @param result The result of the command must be assigned to this
	  *               parameter.
	  * @throws CommandException Thrown when there was an error while
	  *                          executing this command.
	  */
	virtual void execute(const std::vector<TclObject>& tokens,
	                     TclObject& result);

	/** Alternative for the execute() method above.
	  * It has a simpler interface, but performance is a bit lower.
	  * Subclasses should override either this method or the one above.
	  */
	virtual std::string execute(const std::vector<std::string>& tokens);

	/** Attempt tab completion for this command.
	  * Default implementation does nothing.
	  * @param tokens Tokenized command line;
	  *     tokens[0] is the command itself.
	  *     The last token is incomplete, this method tries to complete it.
	  */
	virtual void tabCompletion(std::vector<std::string>& tokens) const;

	// see comments in MSXMotherBoard::loadMachineCommand
	void setAllowedInEmptyMachine(bool value) { allowInEmptyMachine = value; }
	bool isAllowedInEmptyMachine() const { return allowInEmptyMachine; }

protected:
	Command(CommandController& commandController,
	        string_ref name);
	virtual ~Command();

private:
	bool allowInEmptyMachine;
};

} // namespace openmsx

#endif
