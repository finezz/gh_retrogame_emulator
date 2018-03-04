// $Id: MSXRS232.cc 12870 2012-09-21 21:34:15Z manuelbi $

#include "MSXRS232.hh"
#include "RS232Device.hh"
#include "CacheLine.hh"
#include "I8254.hh"
#include "I8251.hh"
#include "Ram.hh"
#include "Rom.hh"
#include "serialize.hh"
#include "unreachable.hh"
#include "BooleanSetting.hh"
#include <cassert>

namespace openmsx {

const unsigned RAM_OFFSET = 0x2000;
const unsigned RAM_SIZE = 0x800;


class Counter0 : public ClockPinListener
{
public:
	explicit Counter0(MSXRS232& rs232);
	virtual ~Counter0();
	virtual void signal(ClockPin& pin, EmuTime::param time);
	virtual void signalPosEdge(ClockPin& pin, EmuTime::param time);
private:
	MSXRS232& rs232;
};

class Counter1 : public ClockPinListener
{
public:
	explicit Counter1(MSXRS232& rs232);
	virtual ~Counter1();
	virtual void signal(ClockPin& pin, EmuTime::param time);
	virtual void signalPosEdge(ClockPin& pin, EmuTime::param time);
private:
	MSXRS232& rs232;
};

class I8251Interf : public I8251Interface
{
public:
	explicit I8251Interf(MSXRS232& rs232);
	virtual ~I8251Interf();
	virtual void setRxRDY(bool status, EmuTime::param time);
	virtual void setDTR(bool status, EmuTime::param time);
	virtual void setRTS(bool status, EmuTime::param time);
	virtual bool getDSR(EmuTime::param time);
	virtual bool getCTS(EmuTime::param time);
	virtual void setDataBits(DataBits bits);
	virtual void setStopBits(StopBits bits);
	virtual void setParityBit(bool enable, ParityBit parity);
	virtual void recvByte(byte value, EmuTime::param time);
	virtual void signal(EmuTime::param time);
private:
	MSXRS232& rs232;
};


MSXRS232::MSXRS232(const DeviceConfig& config)
	: MSXDevice(config)
	, RS232Connector(MSXDevice::getPluggingController(), "msx-rs232")
	, cntr0(new Counter0(*this))
	, cntr1(new Counter1(*this))
	, i8254(new I8254(getScheduler(),
	                  cntr0.get(), cntr1.get(), NULL, getCurrentTime()))
	, interf(new I8251Interf(*this))
	, i8251(new I8251(getScheduler(), *interf, getCurrentTime()))
	, rom(config.findChild("rom")
		? new Rom(MSXDevice::getName() + " ROM", "rom", config)
		: NULL) // when the ROM is already mapped, you don't want to specify it again here
	, ram(config.getChildDataAsBool("ram", false)
	      ? new Ram(config, MSXDevice::getName() + " RAM",
	                "RS232 RAM", RAM_SIZE)
	      : NULL)
	, rxrdyIRQ(getMotherBoard(), MSXDevice::getName() + ".IRQrxrdy")
	, rxrdyIRQlatch(false)
	, rxrdyIRQenabled(false)
	, hasMemoryBasedIo(config.getChildDataAsBool("memorybasedio", false))
	, ioAccessEnabled(!hasMemoryBasedIo)
	, switchSetting(config.getChildDataAsBool("toshiba_rs232c_switch",
		false) ? new BooleanSetting(getCommandController(),
		"toshiba_rs232c_switch", "status of the RS-232C enable switch",
		true) : NULL)
{
	EmuDuration total(1.0 / 1.8432e6); // 1.8432MHz
	EmuDuration hi   (1.0 / 3.6864e6); //   half clock period
	EmuTime::param time = getCurrentTime();
	i8254->getClockPin(0).setPeriodicState(total, hi, time);
	i8254->getClockPin(1).setPeriodicState(total, hi, time);
	i8254->getClockPin(2).setPeriodicState(total, hi, time);

	powerUp(time);
}

MSXRS232::~MSXRS232()
{
}

void MSXRS232::powerUp(EmuTime::param time)
{
	if(ram.get()) {
		ram->clear();
	}
	reset(time);
}

void MSXRS232::reset(EmuTime::param /*time*/)
{
	rxrdyIRQlatch = false;
	rxrdyIRQenabled = false;
	rxrdyIRQ.reset();

	ioAccessEnabled = !hasMemoryBasedIo;

	if (ram.get()) {
		ram->clear();
	}
}

byte MSXRS232::readMem(word address, EmuTime::param time)
{
	if (hasMemoryBasedIo && (0xBFF8 <= address) && (address <= 0xBFFF)) {
		return readIOImpl(address & 0x07, time);
	}
	word addr = address & 0x3FFF;
	if (ram.get() && ((RAM_OFFSET <= addr) && (addr < (RAM_OFFSET + RAM_SIZE)))) {
		return (*ram)[addr - RAM_OFFSET];
	} else if (rom.get() && (0x4000 <= address) && (address < 0x8000)) {
		return (*rom)[addr];
	} else {
		return 0xFF;
	}
}

const byte* MSXRS232::getReadCacheLine(word start) const
{
        if (hasMemoryBasedIo && (start == (0xBFF8 & CacheLine::HIGH))) {
                return NULL;
        }
	word addr = start & 0x3FFF;
	if (ram.get() && ((RAM_OFFSET <= addr) && (addr < (RAM_OFFSET + RAM_SIZE)))) {
		return &(*ram)[addr - RAM_OFFSET];
	} else if (rom.get() && (0x4000 <= start) && (start < 0x8000)) {
		return &(*rom)[addr];
	} else {
		return unmappedRead;
	}
}

void MSXRS232::writeMem(word address, byte value, EmuTime::param time)
{

	if (hasMemoryBasedIo && (0xBFF8 <= address) && (address <= 0xBFFF)) {
		// when the interface has memory based I/O, the I/O port
		// based I/O is disabled, but it can be enabled by writing
		// bit 4 to 0xBFFA. It is disabled again at reset.
		// Source: Sony HB-G900P and Sony HB-G900AP service manuals.
		// We assume here you can also disable it by writing 0 to it.
		if (address == 0xBFFA) {
			ioAccessEnabled = (value & (1 << 4))!=0;
		}
		return writeIOImpl(address & 0x07, value, time);
	}
	word addr = address & 0x3FFF;
	if (ram.get() && ((RAM_OFFSET <= addr) && (addr < (RAM_OFFSET + RAM_SIZE)))) {
		(*ram)[addr - RAM_OFFSET] = value;
	}
}

byte* MSXRS232::getWriteCacheLine(word start) const
{
        if (hasMemoryBasedIo && (start == (0xBFF8 & CacheLine::HIGH))) {
                return NULL;
        }
	word addr = start & 0x3FFF;
	if (ram.get() && ((RAM_OFFSET <= addr) && (addr < (RAM_OFFSET + RAM_SIZE)))) {
		return &(*ram)[addr - RAM_OFFSET];
	} else {
		return unmappedWrite;
	}
}

byte MSXRS232::readIO(word port, EmuTime::param time)
{
	if (ioAccessEnabled) {
		return readIOImpl(port & 0x07, time);
	}
	return 0xFF;
}

byte MSXRS232::readIOImpl(word port, EmuTime::param time)
{
	byte result;
	switch (port) {
		case 0: // UART data register
		case 1: // UART status register
			result = i8251->readIO(port, time);
			break;
		case 2: // Status sense port
			result = readStatus(time);
			break;
		case 3: // no function
			result = 0xFF;
			break;
		case 4: // counter 0 data port
		case 5: // counter 1 data port
		case 6: // counter 2 data port
		case 7: // timer command register
			result = i8254->readIO(port - 4, time);
			break;
		default:
			UNREACHABLE; return 0;
	}
	return result;
}

byte MSXRS232::peekIO(word port, EmuTime::param time) const
{
	if (hasMemoryBasedIo && !ioAccessEnabled) return 0xFF;
	byte result;
	port &= 0x07;
	switch (port) {
		case 0: // UART data register
		case 1: // UART status register
			result = i8251->peekIO(port, time);
			break;
		case 2: // Status sense port
			result = 0; // TODO not implemented
			break;
		case 3: // no function
			result = 0xFF;
			break;
		case 4: // counter 0 data port
		case 5: // counter 1 data port
		case 6: // counter 2 data port
		case 7: // timer command register
			result = i8254->peekIO(port - 4, time);
			break;
		default:
			UNREACHABLE; return 0;
	}
	return result;
}

void MSXRS232::writeIO(word port, byte value, EmuTime::param time)
{
	if (ioAccessEnabled) writeIOImpl(port & 0x07, value, time);
}

void MSXRS232::writeIOImpl(word port, byte value, EmuTime::param time)
{
	switch (port) {
		case 0: // UART data register
		case 1: // UART command register
			i8251->writeIO(port, value, time);
			break;
		case 2: // interrupt mask register
			setIRQMask(value);
			break;
		case 3: // no function
			break;
		case 4: // counter 0 data port
		case 5: // counter 1 data port
		case 6: // counter 2 data port
		case 7: // timer command register
			i8254->writeIO(port - 4, value, time);
			break;
	}
}

byte MSXRS232::readStatus(EmuTime::param time)
{

	// Info from http://nocash.emubase.de/portar.htm
	//
	//  Bit Name  Expl.
	//  0   CD    Carrier Detect   (0=Active, 1=Not active)
	//  1   RI    Ring Indicator   (0=Active, 1=Not active) (N/C in MSX)
	//  6         Timer Output from i8253 Counter 2
	//  7   CTS   Clear to Send    (0=Active, 1=Not active)
	//
	// On Toshiba HX-22, see
	//   http://www.msx.org/forum/msx-talk/hardware/toshiba-hx-22?page=3
	//   RetroTechie's post of 20-09-2012, 08:08
	//   ... The "RS-232 interrupt disable" bit can be read back via bit 3
	//   on this I/O port, if CN1 is open. If CN1 is closed, it always
	//   reads back as "0". ...

	byte result = 0; // TODO check unused bits

	// TODO bit 0: carrier detect

	if (!rxrdyIRQenabled && switchSetting.get() &&
			switchSetting->getValue()) {
		result |= 0x08;
	}

	if (!interf->getCTS(time)) {
		result |= 0x80;
	}
	if (i8254->getOutputPin(2).getState(time)) {
		result |= 0x40;
	}
	return result;
}

void MSXRS232::setIRQMask(byte value)
{
	enableRxRDYIRQ(!(value & 1));
}

void MSXRS232::setRxRDYIRQ(bool status)
{
	if (rxrdyIRQlatch != status) {
		rxrdyIRQlatch = status;
		if (rxrdyIRQenabled) {
			if (rxrdyIRQlatch) {
				rxrdyIRQ.set();
			} else {
				rxrdyIRQ.reset();
			}
		}
	}
}

void MSXRS232::enableRxRDYIRQ(bool enabled)
{
	if (rxrdyIRQenabled != enabled) {
		rxrdyIRQenabled = enabled;
		if (!rxrdyIRQenabled && rxrdyIRQlatch) {
			rxrdyIRQ.reset();
		}
	}
}


// I8251Interface  (pass calls from I8251 to outConnector)

I8251Interf::I8251Interf(MSXRS232& rs232_)
	: rs232(rs232_)
{
}

I8251Interf::~I8251Interf()
{
}

void I8251Interf::setRxRDY(bool status, EmuTime::param /*time*/)
{
	rs232.setRxRDYIRQ(status);
}

void I8251Interf::setDTR(bool status, EmuTime::param time)
{
	rs232.getPluggedRS232Dev().setDTR(status, time);
}

void I8251Interf::setRTS(bool status, EmuTime::param time)
{
	rs232.getPluggedRS232Dev().setRTS(status, time);
}

bool I8251Interf::getDSR(EmuTime::param time)
{
	return rs232.getPluggedRS232Dev().getDSR(time);
}

bool I8251Interf::getCTS(EmuTime::param time)
{
	return rs232.getPluggedRS232Dev().getCTS(time);
}

void I8251Interf::setDataBits(DataBits bits)
{
	rs232.getPluggedRS232Dev().setDataBits(bits);
}

void I8251Interf::setStopBits(StopBits bits)
{
	rs232.getPluggedRS232Dev().setStopBits(bits);
}

void I8251Interf::setParityBit(bool enable, ParityBit parity)
{
	rs232.getPluggedRS232Dev().setParityBit(enable, parity);
}

void I8251Interf::recvByte(byte value, EmuTime::param time)
{
	rs232.getPluggedRS232Dev().recvByte(value, time);
}

void I8251Interf::signal(EmuTime::param time)
{
	rs232.getPluggedRS232Dev().signal(time); // for input
}


// Counter 0 output

Counter0::Counter0(MSXRS232& rs232_)
	: rs232(rs232_)
{
}

Counter0::~Counter0()
{
}

void Counter0::signal(ClockPin& pin, EmuTime::param time)
{
	ClockPin& clk = rs232.i8251->getClockPin();
	if (pin.isPeriodic()) {
		clk.setPeriodicState(pin.getTotalDuration(),
		                     pin.getHighDuration(), time);
	} else {
		clk.setState(pin.getState(time), time);
	}
}

void Counter0::signalPosEdge(ClockPin& /*pin*/, EmuTime::param /*time*/)
{
	UNREACHABLE;
}


// Counter 1 output // TODO split rx tx

Counter1::Counter1(MSXRS232& rs232_)
	: rs232(rs232_)
{
}

Counter1::~Counter1()
{
}

void Counter1::signal(ClockPin& pin, EmuTime::param time)
{
	ClockPin& clk = rs232.i8251->getClockPin();
	if (pin.isPeriodic()) {
		clk.setPeriodicState(pin.getTotalDuration(),
		                     pin.getHighDuration(), time);
	} else {
		clk.setState(pin.getState(time), time);
	}
}

void Counter1::signalPosEdge(ClockPin& /*pin*/, EmuTime::param /*time*/)
{
	UNREACHABLE;
}


// RS232Connector input

bool MSXRS232::ready()
{
	return i8251->isRecvReady();
}

bool MSXRS232::acceptsData()
{
	return i8251->isRecvEnabled();
}

void MSXRS232::setDataBits(DataBits bits)
{
	i8251->setDataBits(bits);
}

void MSXRS232::setStopBits(StopBits bits)
{
	i8251->setStopBits(bits);
}

void MSXRS232::setParityBit(bool enable, ParityBit parity)
{
	i8251->setParityBit(enable, parity);
}

void MSXRS232::recvByte(byte value, EmuTime::param time)
{
	i8251->recvByte(value, time);
}

// version 1: initial version
// version 2: added ioAccessEnabled
// TODO: serialize switch status?
template<typename Archive>
void MSXRS232::serialize(Archive& ar, unsigned version)
{
	ar.template serializeBase<MSXDevice>(*this);
	ar.template serializeBase<RS232Connector>(*this);

	ar.serialize("I8254", *i8254);
	ar.serialize("I8251", *i8251);
	if (ram.get()) {
		ar.serialize("ram", *ram);
	}
	ar.serialize("rxrdyIRQ", rxrdyIRQ);
	ar.serialize("rxrdyIRQlatch", rxrdyIRQlatch);
	ar.serialize("rxrdyIRQenabled", rxrdyIRQenabled);
	if (ar.versionAtLeast(version, 2)) {
		ar.serialize("ioAccessEnabled", ioAccessEnabled);
	} else {
		assert(ar.isLoader());
		ioAccessEnabled = !hasMemoryBasedIo; // we can't know the
					// actual value, but this is probably
					// safest
	}

	// don't serialize cntr0, cntr1, interf
}
INSTANTIATE_SERIALIZE_METHODS(MSXRS232);
REGISTER_MSXDEVICE(MSXRS232, "RS232");

} // namespace openmsx

