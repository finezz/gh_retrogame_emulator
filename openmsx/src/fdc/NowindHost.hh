// $Id: NowindHost.hh 12018 2011-03-13 10:05:58Z mthuurne $

#ifndef NOWINDHOST_HH
#define NOWINDHOST_HH

#include "openmsx.hh"
#include <deque>
#include <vector>
#include <string>
#include <memory>
#include <iosfwd>

namespace openmsx {

class DiskContainer;
class SectorAccessibleDisk;

class NowindHost
{
public:
	explicit NowindHost(const std::vector<DiskContainer*>& drives);
	~NowindHost();

	// public for usb-host implementation
	bool isDataAvailable() const;

	// read one byte of response-data from the host (msx <- pc)
	byte read();

	// like read(), but without side effects (doesn't consume the data)
	byte peek() const;

	// Write one byte of command-data to the host   (msx -> pc)
	// Time parameter is in milliseconds. Emulators can pass emulation
	// time, usbhost can pass real time.
	void write(byte value, unsigned time);

	void setAllowOtherDiskroms(bool allow);
	bool getAllowOtherDiskroms() const;

	void setEnablePhantomDrives(bool enable);
	bool getEnablePhantomDrives() const;

	template<typename Archive>
	void serialize(Archive& ar, unsigned version);

	// public for serialization
	enum State {
		STATE_SYNC1,     // waiting for AF
		STATE_SYNC2,     // waiting for 05
		STATE_COMMAND,   // waiting for command (9 bytes)
		STATE_DISKREAD,  // waiting for AF07
		STATE_DISKWRITE, // waiting for AA<data>AA
		STATE_DEVOPEN,   // waiting for filename (11 bytes)
		STATE_IMAGE,     // waiting for filename
		STATE_MESSAGE,   // waiting for null-terminated message
	};

private:
	void msxReset();
	SectorAccessibleDisk* getDisk() const;
	void executeCommand();

	void send(byte value);
	void send16(word value);
	void sendHeader();
	void purge();

	void DRIVES();
	void DSKCHG();
	void CHOICE();
	void INIENV();
	void setDateMSX();

	unsigned getSectorAmount() const;
	unsigned getStartSector() const;
	unsigned getStartAddress() const;
	unsigned getCurrentAddress() const;

	void diskReadInit(SectorAccessibleDisk& disk);
	void doDiskRead1();
	void doDiskRead2();
	void transferSectors(unsigned transferAddress, unsigned amount);
	void transferSectorsBackwards(unsigned transferAddress, unsigned amount);

	void diskWriteInit(SectorAccessibleDisk& disk);
	void doDiskWrite1();
	void doDiskWrite2();

	unsigned getFCB() const;
	std::string extractName(int begin, int end) const;
	unsigned readHelper1(unsigned dev, char* buffer);
	void readHelper2(unsigned len, const char* buffer);
	int getDeviceNum() const;
	int getFreeDeviceNum();
	void deviceOpen();
	void deviceClose();
	void deviceWrite();
	void deviceRead();

	void callImage(const std::string& filename);


	static const unsigned MAX_DEVICES = 16;

	const std::vector<DiskContainer*>& drives;

	std::deque<byte> hostToMsxFifo;

	struct {
		std::auto_ptr<std::fstream> fs; // not in use when fs == NULL
		unsigned fcb;
	} devices[MAX_DEVICES];

	// state-machine
	std::vector<byte> buffer;// work buffer for diskread/write
	unsigned lastTime;       // last time a byte was received from MSX
	State state;
	unsigned recvCount;      // how many bytes recv in this state
	unsigned transfered;     // progress within diskread/write
	unsigned retryCount;     // only used for diskread
	unsigned transferSize;   // size of current chunk
	byte cmdData[9];         // reg_[cbedlhfa] + cmd
	byte extraData[240 + 2]; // extra data for diskread/write

	byte romdisk;            // index of romdisk (255 = no romdisk)
	bool allowOtherDiskroms;
	bool enablePhantomDrives;
};

} // namespace openmsx

#endif // NOWINDHOST_HH
