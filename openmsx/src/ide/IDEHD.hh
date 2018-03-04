// $Id: IDEHD.hh 12527 2012-05-17 17:34:11Z m9710797 $

#ifndef IDEHD_HH
#define IDEHD_HH

#include "HD.hh"
#include "AbstractIDEDevice.hh"
#include "serialize_meta.hh"
#include "noncopyable.hh"

namespace openmsx {

class DeviceConfig;
class DiskManipulator;

class IDEHD : public HD, public AbstractIDEDevice, private noncopyable
{
public:
	explicit IDEHD(const DeviceConfig& config);
	virtual ~IDEHD();

	template<typename Archive>
	void serialize(Archive& ar, unsigned version);

private:
	// AbstractIDEDevice:
	virtual bool isPacketDevice();
	virtual const std::string& getDeviceName();
	virtual void fillIdentifyBlock(byte* buffer);
	virtual unsigned readBlockStart(byte* buffer, unsigned count);
	virtual void writeBlockComplete(byte* buffer, unsigned count);
	virtual void executeCommand(byte cmd);

	DiskManipulator& diskManipulator;
	unsigned transferSectorNumber;
};

} // namespace openmsx

#endif
