// $Id: YM2148.hh 8209 2008-09-09 20:48:44Z m9710797 $
/* Ported from:
** Source: /cvsroot/bluemsx/blueMSX/Src/Memory/romMapperSfg05.c,v
** Revision: 1.12
** Date: 2007/04/28 05:06:29
**
** More info: http://www.bluemsx.com
**
** Copyright (C) 2003-2006 Daniel Vik
*/

// SFG05 Midi: The MIDI Out is probably buffered. If UART is unbuffered, all
//             data will not be transmitted correctly. Question is how big
//             the buffer is.
//             The command bits are not clear at all. Only known bit is the
//             reset bit.

// NOTES: Cmd bit 3: seems to be enable/disable something (checked before RX
//        Cmd bit 4: is set when IM2 is used, cleared when IM1 is used


#ifndef YM2148_HH
#define YM2148_HH

#include "openmsx.hh"

namespace openmsx {

class YM2148
{
public:
	YM2148();
	~YM2148();
	void reset();

	byte readStatus();
	byte readData();
	void setVector(byte value);
	void writeCommand(byte value);
	void writeData(byte value);

	template<typename Archive>
	void serialize(Archive& ar, unsigned version);

private:
	void midiInCallback(byte* buffer, unsigned length);
	void onRecv();
	void onTrans();

	//TODO MidiIO*     midiIo;
	//TODO BoardTimer* timerRecv;
	//TODO BoardTimer* timerTrans;
	//TODO void*       semaphore;
	static const unsigned RX_QUEUE_SIZE = 256;
	int      txPending;
	int      rxPending;
	int      rxHead;
	unsigned charTime;
	unsigned timeRecv;
	unsigned timeTrans;
	byte     command;
	byte     rxData;
	byte     status;
	byte     txBuffer;
	byte     rxQueue[RX_QUEUE_SIZE];
	byte     vector;
};

} // namespace openmsx

#endif
