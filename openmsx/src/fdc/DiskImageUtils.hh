// $Id: DiskImageUtils.hh 12825 2012-08-21 17:05:28Z m9710797 $

#ifndef DISK_IMAGE_UTILS_HH
#define DISK_IMAGE_UTILS_HH

#include "openmsx.hh"
#include "endian.hh"
#include "alignof.hh"
#include "static_assert.hh"
#include <vector>
#include <memory>

namespace openmsx {

class SectorAccessibleDisk;

// TODO if we can guarantee this whole struct will be aligned, then we can use
//      the more efficient Endian:::L16 type
struct MSXBootSector {
	byte           jumpCode[3];   // + 0 0xE5 to bootprogram
	byte           name[8];       // + 3
	Endian::UA_L16 bpSector;      // +11 bytes per sector (always 512)
	byte           spCluster;     // +13 sectors per cluster (always 2)
	Endian::UA_L16 resvSectors;   // +14 nb of non-data sectors (ex bootsector) // TODO aligned
	byte           nrFats;        // +16 nb of fats
	Endian::UA_L16 dirEntries;    // +17 max nb of files in root directory
	Endian::UA_L16 nrSectors;     // +19 nb of sectors on this disk
	byte           descriptor;    // +21 media descriptor
	Endian::UA_L16 sectorsFat;    // +22 sectors per FAT   // TODO aligned
	Endian::UA_L16 sectorsTrack;  // +24 sectors per track // TODO aligned
	Endian::UA_L16 nrSides;       // +26 number of side    // TODO aligned
	Endian::UA_L16 hiddenSectors; // +28 not use           // TODO aligned
	byte           bootProg[512-30];// +30 actual bootprogram
};
STATIC_ASSERT(sizeof(MSXBootSector) == 512);
STATIC_ASSERT(ALIGNOF(MSXBootSector) == 1); // TODO don't require this in the future

// TODO aligned, see above
struct MSXDirEntry {
	char           filename[8];  // + 0
	char           ext[3];       // + 8
	byte           attrib;       // +11
	byte           reserved[10]; // +12 unused
	Endian::UA_L16 time;         // +22 // TODO aligned
	Endian::UA_L16 date;         // +24 // TODO aligned
	Endian::UA_L16 startCluster; // +26 // TODO aligned
	Endian::UA_L32 size;         // +28 // TODO aligned
};
STATIC_ASSERT(sizeof(MSXDirEntry) == 32);
STATIC_ASSERT(ALIGNOF(MSXDirEntry) == 1); // TODO don't require this in the future

// Note: can't use Endian::L32 for 'start' and 'size' because the Partition
//       struct itself is not 4-bytes aligned.
struct Partition {
	byte           boot_ind;   // + 0 0x80 - active
	byte           head;       // + 1 starting head
	byte           sector;     // + 2 tarting sector
	byte           cyl;        // + 3 starting cylinder
	byte           sys_ind;    // + 4 what partition type
	byte           end_head;   // + 5 end head
	byte           end_sector; // + 6 end sector
	byte           end_cyl;    // + 7 end cylinder
	Endian::UA_L32 start;      // + 8 starting sector counting from 0
	Endian::UA_L32 size;       // +12 nr of sectors in partition
};
STATIC_ASSERT(sizeof(Partition) == 16);
STATIC_ASSERT(ALIGNOF(Partition) == 1);

struct PartitionTable {
	char      header[11]; // +  0
	char      pad[3];     // +  3
	Partition part[31];   // + 14,+30,..,+494    Not 4-byte aligned!!
	byte      end[2];     // +510
};
STATIC_ASSERT(sizeof(PartitionTable) == 512);


namespace DiskImageUtils {

	/** Checks whether
	 *   the disk is partitioned
	 *   the specified partition exists
	 * throws a CommandException if one of these conditions is false
	 * @param disk The disk to check.
	 * @param partition Partition number, in range [1..31].
	 */
	void checkValidPartition(SectorAccessibleDisk& disk, unsigned partition);

	/** Like above, but also check whether partition is of type FAT12.
	 */
	void checkFAT12Partition(SectorAccessibleDisk& disk, unsigned partition);

	/** Check whether the given disk is partitioned.
	 */
	bool hasPartitionTable(SectorAccessibleDisk& disk);

	/** Format the given disk (= a single partition).
	 * The formatting depends on the size of the image.
	 */
	void format(SectorAccessibleDisk& disk);

	/** Write a partition table to the given disk and format each partition
	 * @param disk The disk to partition.
	 * @param sizes The number of sectors for each partition.
	 */
	void partition(SectorAccessibleDisk& disk,
	               const std::vector<unsigned>& sizes);
};

} // namespace openmsx

#endif
