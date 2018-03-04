// $Id: WD33C93.hh 12527 2012-05-17 17:34:11Z m9710797 $
/* Ported from:
** Source: /cvsroot/bluemsx/blueMSX/Src/IoDevice/wd33c93.h,v
** Revision: 1.6
** Date: 2007/03/22 10:55:08
**
** More info: http://www.bluemsx.com
**
** Copyright (C) 2003-2007 Daniel Vik, Ricardo Bittencourt, white cat
*/

#ifndef WD33C93_HH
#define WD33C93_HH

#include "SCSI.hh"
#include "MemBuffer.hh"
#include <memory>

namespace openmsx {

class SCSIDevice;
class DeviceConfig;

class WD33C93
{
public:
	explicit WD33C93(const DeviceConfig& config);
	~WD33C93();

	void reset(bool scsireset);

	byte readAuxStatus();
	byte readCtrl();
	byte peekAuxStatus() const;
	byte peekCtrl() const;
	void writeAdr(byte value);
	void writeCtrl(byte value);

	template<typename Archive>
	void serialize(Archive& ar, unsigned version);

private:
	void disconnect();
	void execCmd(byte value);

	MemBuffer<byte> buffer;
	std::auto_ptr<SCSIDevice> dev[8];
	unsigned bufIdx;
	int counter;
	unsigned blockCounter;
	int tc;
	SCSI::Phase phase;
	byte myId;
	byte targetId;
	byte regs[32];
	byte latch;
	bool devBusy;
};

} // namespace openmsx

#endif
