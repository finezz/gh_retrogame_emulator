// $Id: NowindCommand.hh 12609 2012-06-12 21:07:44Z m9710797 $

#ifndef NOWINDCOMMAND_HH
#define NOWINDCOMMAND_HH

#include "NowindInterface.hh"
#include "Command.hh"

namespace openmsx {

class DiskChanger;
class MSXMotherBoard;

class NowindCommand : public Command
{
public:
	NowindCommand(const std::string& basename,
	              CommandController& commandController,
	              NowindInterface& interface);
	virtual std::string execute(const std::vector<std::string>& tokens);
	virtual std::string help(const std::vector<std::string>& tokens) const;
	virtual void tabCompletion(std::vector<std::string>& tokens) const;

	DiskChanger* createDiskChanger(const std::string& basename, unsigned n,
	                               MSXMotherBoard& motherBoard) const;

private:
	unsigned searchRomdisk(const NowindInterface::Drives& drives) const;
	void processHdimage(const std::string& hdimage,
	                    NowindInterface::Drives& drives) const;
	NowindInterface& interface;
};

} // namespace openmsx

#endif
