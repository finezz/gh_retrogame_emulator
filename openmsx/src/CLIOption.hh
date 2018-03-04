// $Id: CLIOption.hh 12628 2012-06-14 20:15:37Z m9710797 $

#ifndef CLIOPTION_HH
#define CLIOPTION_HH

#include "string_ref.hh"
#include <deque>

namespace openmsx {

class CLIOption
{
public:
	virtual ~CLIOption() {}
	virtual bool parseOption(const std::string& option,
	                         std::deque<std::string>& cmdLine) = 0;
	virtual string_ref optionHelp() const = 0;

protected:
	std::string getArgument(const std::string& option,
	                        std::deque<std::string>& cmdLine) const;
	std::string peekArgument(const std::deque<std::string>& cmdLine) const;
};

class CLIFileType
{
public:
	virtual ~CLIFileType() {}
	virtual void parseFileType(const std::string& filename,
	                           std::deque<std::string>& cmdLine) = 0;
	virtual string_ref fileTypeHelp() const = 0;
};

} // namespace openmsx

#endif
