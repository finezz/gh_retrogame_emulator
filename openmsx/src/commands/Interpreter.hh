// $Id: Interpreter.hh 12809 2012-08-13 19:27:49Z m9710797 $

#ifndef INTERPRETER_HH
#define INTERPRETER_HH

#include "TclParser.hh"
#include "EventListener.hh"
#include "StringMap.hh"
#include "string_ref.hh"
#include "noncopyable.hh"
#include "vla.hh"
#include <set>
#include <vector>
#include <tcl.h>

namespace openmsx {

class EventDistributor;
class Command;
class Setting;
class InterpreterOutput;
class TclObject;

class Interpreter : private EventListener, private noncopyable
{
public:
	explicit Interpreter(EventDistributor& eventDistributor);
	~Interpreter();

	void setOutput(InterpreterOutput* output);

	void init(const char* programName);
	void registerCommand(const std::string& name, Command& command);
	void unregisterCommand(string_ref name, Command& command);
	void getCommandNames(std::set<std::string>& result);
	bool isComplete(const std::string& command) const;
	std::string execute(const std::string& command);
	std::string executeFile(const std::string& filename);

	void setVariable(const std::string& name, const std::string& value);
	void unsetVariable(const std::string& name);
	const char* getVariable(const std::string& name) const;
	void registerSetting(Setting& variable, const std::string& name);
	void unregisterSetting(Setting& variable, const std::string& name);

	/** Create the global namespace with given name.
	  * @param name Name of the namespace, should not include '::' prefix.
	  */
	void createNamespace(const std::string& name);

	/** Delete the global namespace with given name.
	  * @param name Name of the namespace, should not include '::' prefix.
	  */
	void deleteNamespace(const std::string& name);

	void splitList(const std::string& list,
	               std::vector<std::string>& result);
	static void splitList(const std::string& list,
	                      std::vector<std::string>& result,
	                      Tcl_Interp* interp);

	TclParser parse(string_ref command);

private:
	// EventListener
	virtual int signalEvent(const shared_ptr<const Event>& event);

	void poll();

	static int outputProc(ClientData clientData, const char* buf,
	        int toWrite, int* errorCodePtr);
	static int commandProc(ClientData clientData, Tcl_Interp* interp,
	                       int objc, Tcl_Obj* const objv[]);
	static char* traceProc(ClientData clientData, Tcl_Interp* interp,
	                       const char* part1, const char* part2, int flags);

	EventDistributor& eventDistributor;

	static Tcl_ChannelType channelType;
	Tcl_Interp* interp;
	typedef StringMap<Tcl_Command> CommandTokenMap;
	CommandTokenMap commandTokenMap;
	InterpreterOutput* output;

	friend class TclObject;
};

} // namespace openmsx

#endif
