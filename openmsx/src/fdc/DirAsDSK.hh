// $Id: DirAsDSK.hh 12858 2012-09-09 08:20:17Z m9710797 $

#ifndef DIRASDSK_HH
#define DIRASDSK_HH

#include "SectorBasedDisk.hh"
#include "DiskImageUtils.hh"
#include "EmuTime.hh"
#include <sys/types.h>

struct stat;

namespace openmsx {

class DiskChanger;
class CliComm;

class DirAsDSK : public SectorBasedDisk
{
public:
	enum SyncMode { SYNC_READONLY, SYNC_FULL };
	enum BootSectorType { BOOTSECTOR_DOS1, BOOTSECTOR_DOS2 };

	static const unsigned SECTORS_PER_FAT = 3;
	static const unsigned SECTORS_PER_DIR = 7;
	static const unsigned DIR_ENTRIES_PER_SECTOR =
	       SECTOR_SIZE / sizeof(MSXDirEntry);
	static const unsigned NUM_DIR_ENTRIES =
		SECTORS_PER_DIR * DIR_ENTRIES_PER_SECTOR;
	static const unsigned NUM_SECTORS = 1440;

public:
	DirAsDSK(DiskChanger& diskChanger, CliComm& cliComm,
	         const Filename& hostDir, SyncMode syncMode,
	         BootSectorType bootSectorType);

	// SectorBasedDisk
	virtual void readSectorImpl(unsigned sector, byte* buf);
	virtual void writeSectorImpl(unsigned sector, const byte* buf);
	virtual bool isWriteProtectedImpl() const;
	virtual void checkCaches();

private:
	byte* fat();
	byte* fat2();
	MSXDirEntry& msxDir(unsigned dirIndex);
	void writeFATSector (unsigned sector, const byte* buf);
	void writeDIRSector (unsigned sector, const byte* buf);
	void writeDataSector(unsigned sector, const byte* buf);
	void writeDIREntry(unsigned dirIndex, const MSXDirEntry& newEntry);
	void syncWithHost();
	void checkDeletedHostFiles();
	void deleteMSXFile(unsigned dirIndex);
	void freeFATChain(unsigned curCl);
	void addNewHostFiles();
	void foundNewHostFile(const std::string& hostName);
	bool checkFileUsedInDSK(const std::string& hostName);
	bool checkMSXFileExists(const std::string& msxfilename);
	void checkModifiedHostFile(unsigned dirIndex);
	void importHostFile(unsigned dirIndex, struct stat& fst);
	void exportToHostFile(unsigned dirIndex);
	unsigned findNextFreeCluster(unsigned curcl);
	unsigned findFirstFreeCluster();
	unsigned readFAT(unsigned clnr);
	void writeFAT12(unsigned clnr, unsigned val);
	void exportFileFromFATChange(unsigned cluster, byte* oldFAT);
	unsigned getChainStart(unsigned cluster, unsigned& chainLength);
	unsigned getDirEntryForCluster(unsigned cluster);

private:
	DiskChanger& diskChanger; // used to query time / report disk change
	CliComm& cliComm; // TODO don't use CliComm to report errors/warnings
	const std::string hostDir;
	const SyncMode syncMode;

	EmuTime lastAccess; // last time there was a sector read/write

	// Storage for the whole virtual disk.
	byte sectors[NUM_SECTORS][SECTOR_SIZE];

	// For each directory entry we store the name of the corresponding
	// host file, and the last modification time (and filesize) of the host
	// file.
	struct {
		bool inUse() const { return !hostName.empty(); }

		std::string hostName; // path relative to 'hostDir'
		// The following two are used to detect changes in the host
		// file compared to the last host->virtual-disk sync.
		time_t mtime; // Modification time of host file at the time of
		              // the last sync.
		int filesize; // Host file size, normally the same as msx
		              // filesize, except when the host file was
		              // truncated.
	} mapDir[NUM_DIR_ENTRIES];
};

} // namespace openmsx

#endif
