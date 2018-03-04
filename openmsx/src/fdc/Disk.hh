// $Id: Disk.hh 12446 2012-03-24 16:35:59Z m9710797 $

#ifndef DISK_HH
#define DISK_HH

#include "SectorAccessibleDisk.hh"
#include "RawTrack.hh"
#include "DiskName.hh"
#include "openmsx.hh"

namespace openmsx {

class Disk : public SectorAccessibleDisk
{
public:
	virtual ~Disk();

	const DiskName& getName() const;

	/** Replace a full track in this image with the given track. */
	        void writeTrack(byte track, byte side, const RawTrack& input);

	/** Read a full track from this disk image. */
	virtual void readTrack (byte track, byte side,       RawTrack& output) = 0;

	bool isDoubleSided();

protected:
	explicit Disk(const DiskName& name);
	int physToLog(byte track, byte side, byte sector);
	void logToPhys(int log, byte& track, byte& side, byte& sector);

	virtual void detectGeometry();
	virtual void detectGeometryFallback();

	void setSectorsPerTrack(unsigned num);
	unsigned getSectorsPerTrack();
	void setNbSides(unsigned num);

	virtual void writeTrackImpl(byte track, byte side, const RawTrack& input) = 0;

private:
	const DiskName name;
	unsigned sectorsPerTrack;
	unsigned nbSides;
};

} // namespace openmsx

#endif
