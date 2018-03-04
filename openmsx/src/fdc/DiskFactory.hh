// $Id: DiskFactory.hh 12845 2012-09-08 08:10:15Z m9710797 $

#ifndef DISKFACTORY_HH
#define DISKFACTORY_HH

#include "DirAsDSK.hh"
#include <string>

namespace openmsx {

class Reactor;
class DiskChanger;
class Disk;
template <class T> class EnumSetting;

class DiskFactory
{
public:
	explicit DiskFactory(Reactor& reactor);
	Disk* createDisk(const std::string& diskImage, DiskChanger& diskChanger);

private:
	Reactor& reactor;
	std::auto_ptr<EnumSetting<DirAsDSK::BootSectorType> > bootSectorSetting;
	std::auto_ptr<EnumSetting<DirAsDSK::SyncMode> > syncDirAsDSKSetting;
};

} // namespace openmsx

#endif // DISKFACTORY_HH
