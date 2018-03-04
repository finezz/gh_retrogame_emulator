// $Id: RomDatabase.hh 12587 2012-06-06 20:31:32Z m9710797 $

#ifndef ROMDATABASE_HH
#define ROMDATABASE_HH

#include "sha1.hh"
#include "noncopyable.hh"
#include <string>
#include <memory>
#include <map>

namespace openmsx {

class CliComm;
class RomInfo;
class SoftwareInfoTopic;
class GlobalCommandController;

class RomDatabase : private noncopyable
{
public:
	typedef std::map<Sha1Sum, RomInfo*> DBMap;

	RomDatabase(GlobalCommandController& commandController, CliComm& cliComm);
	~RomDatabase();

	/** Lookup an entry in the database by sha1sum.
	 * Returns NULL when no corresponding entry was found.
	 */
	const RomInfo* fetchRomInfo(const Sha1Sum& sha1sum) const;

private:
	DBMap romDBSHA1;
	std::auto_ptr<SoftwareInfoTopic> softwareInfoTopic;
};

} // namespace openmsx

#endif
