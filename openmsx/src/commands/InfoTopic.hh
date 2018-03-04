// $Id: InfoTopic.hh 12805 2012-08-13 19:26:18Z m9710797 $

#ifndef INFOTOPIC_HH
#define INFOTOPIC_HH

#include "Completer.hh"
#include <string>
#include <vector>

namespace openmsx {

class TclObject;
class InfoCommand;

class InfoTopic : public Completer
{
public:
	/** Show info on this topic
	  * @param tokens Tokenized command line;
	  *     tokens[1] is the topic.
	  * @param result The result of this topic must be assigned to this
	  *               parameter.
	  * @throw CommandException Thrown when there was an error while
	  *                         executing this InfoTopic.
	  */
	virtual void execute(const std::vector<TclObject>& tokens,
	                     TclObject& result) const = 0;

	/** Print help for this topic.
	  * @param tokens Tokenized command line;
	  *     tokens[1] is the topic.
	  */
	virtual std::string help(const std::vector<std::string>& tokens) const = 0;

	/** Attempt tab completion for this topic.
	  * Default implementation does nothing.
	  * @param tokens Tokenized command line;
	  *     tokens[1] is the topic.
	  *     The last token is incomplete, this method tries to complete it.
	  */
	virtual void tabCompletion(std::vector<std::string>& tokens) const;

protected:
	InfoTopic(InfoCommand& infoCommand, const std::string& name);
	virtual ~InfoTopic();

private:
	InfoCommand& infoCommand;
};

} // namespace openmsx

#endif
