// $Id: DummySCSIDevice.hh 8237 2008-09-18 17:54:54Z m9710797 $
#ifndef DUMMYSCSIDEVICE_HH
#define DUMMYSCSIDEVICE_HH

#include "SCSIDevice.hh"
#include "serialize_meta.hh"

namespace openmsx {

class DummySCSIDevice : public SCSIDevice
{
public:
	virtual void reset();
	virtual bool isSelected();
	virtual unsigned executeCmd(const byte* cdb, SCSI::Phase& phase,
	                            unsigned& blocks);
	virtual unsigned executingCmd(SCSI::Phase& phase, unsigned& blocks);
	virtual byte getStatusCode();
	virtual int msgOut(byte value);
	virtual byte msgIn();
	virtual void disconnect();
	virtual void busReset(); // only used in MB89352 controller

	virtual unsigned dataIn(unsigned& blocks);
	virtual unsigned dataOut(unsigned& blocks);

	template<typename Archive>
	void serialize(Archive& ar, unsigned version);
};

} // namespace openmsx

#endif
