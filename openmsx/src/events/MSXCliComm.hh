// $Id: MSXCliComm.hh 12625 2012-06-14 20:13:15Z m9710797 $

#ifndef MSXCLICOMM_HH
#define MSXCLICOMM_HH

#include "CliComm.hh"
#include "StringMap.hh"
#include "noncopyable.hh"

namespace openmsx {

class MSXMotherBoard;
class GlobalCliComm;

class MSXCliComm : public CliComm, private noncopyable
{
public:
	MSXCliComm(MSXMotherBoard& motherBoard, GlobalCliComm& cliComm);

	virtual void log(LogLevel level, string_ref message);
	virtual void update(UpdateType type, string_ref name,
	                    string_ref value);

private:
	MSXMotherBoard& motherBoard;
	GlobalCliComm& cliComm;

	typedef StringMap<std::string> PrevValue;
	PrevValue prevValues[NUM_UPDATES];
};

} // namespace openmsx

#endif
