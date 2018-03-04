// $Id: Completer.hh 12627 2012-06-14 20:14:52Z m9710797 $

#ifndef COMPLETER_HH
#define COMPLETER_HH

#include "noncopyable.hh"
#include "string_ref.hh"
#include <vector>
#include <set>

namespace openmsx {

class FileContext;
class InterpreterOutput;

class Completer : private noncopyable
{
public:
	const std::string& getName() const;

	/** Print help for this command.
	  */
	virtual std::string help(const std::vector<std::string>& tokens) const = 0;

	/** Attempt tab completion for this command.
	  * @param tokens Tokenized command line;
	  *     tokens[0] is the command itself.
	  *     The last token is incomplete, this method tries to complete it.
	  */
	virtual void tabCompletion(std::vector<std::string>& tokens) const = 0;

	static void completeString(std::vector<std::string>& tokens,
	                           std::set<std::string>& set,
	                           bool caseSensitive = true);
	static void completeFileName(std::vector<std::string>& tokens,
                                     const FileContext& context);
	static void completeFileName(std::vector<std::string>& tokens,
	                             const FileContext& context,
	                             const std::set<std::string>& extra);

	// should only be called by CommandConsole
	static void setOutput(InterpreterOutput* output);

protected:
	explicit Completer(string_ref name);
	virtual ~Completer();

private:
	static bool completeString2(std::string& str, std::set<std::string>& st,
	                            bool caseSensitive);

	const std::string name;
	static InterpreterOutput* output;
};

} // namespace openmsx

#endif
