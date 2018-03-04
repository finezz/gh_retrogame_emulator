// $Id: DiskName.hh 11877 2011-01-08 20:13:19Z m9710797 $

#ifndef DISKNAME_HH
#define DISKNAME_HH

#include "Filename.hh"

namespace openmsx {

class DiskName
{
public:
	DiskName(const Filename& name, const std::string& extra = "");

	std::string getOriginal() const;
	std::string getResolved() const;
	void updateAfterLoadState();
	bool empty() const;
	const Filename& getFilename() const;

	template<typename Archive>
	void serialize(Archive& ar, unsigned version);

private:
	Filename name;
	std::string extra;
};

} // namespace openmsx

#endif
