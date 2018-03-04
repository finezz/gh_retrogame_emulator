// $Id: DiskName.cc 11877 2011-01-08 20:13:19Z m9710797 $

#include "DiskName.hh"
#include "serialize.hh"

using std::string;

namespace openmsx {

DiskName::DiskName(const Filename& name_, const string& extra_)
	: name(name_)
	, extra(extra_)
{
}

string DiskName::getOriginal() const
{
	return name.getOriginal() + extra;
}

string DiskName::getResolved() const
{
	return name.getResolved() + extra;
}

void DiskName::updateAfterLoadState()
{
	name.updateAfterLoadState();
}

bool DiskName::empty() const
{
	return name.empty() && extra.empty();
}

const Filename& DiskName::getFilename() const
{
	return name;
}

template<typename Archive>
void DiskName::serialize(Archive& ar, unsigned /*version*/)
{
	ar.serialize("filename", name);
	ar.serialize("extra", extra);
}
INSTANTIATE_SERIALIZE_METHODS(DiskName);

} // namespace openmsx

