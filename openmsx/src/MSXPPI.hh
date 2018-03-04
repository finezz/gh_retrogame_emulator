// $Id: MSXPPI.hh 12527 2012-05-17 17:34:11Z m9710797 $

// This class implements the PPI (8255)
//
//   PPI    MSX-I/O  Direction  MSX-Function
//  PortA    0xA8      Out     Memory primary slot register
//  PortB    0xA9      In      Keyboard column inputs
//  PortC    0xAA      Out     Keyboard row select / CAPS / CASo / CASm / SND
//  Control  0xAB     In/Out   Mode select for PPI
//
//  Direction indicates the direction normally used on MSX.
//  Reading from an output port returns the last written byte.
//  Writing to an input port has no immediate effect.
//
//  PortA combined with upper half of PortC form groupA
//  PortB               lower                    groupB
//  GroupA can be in programmed in 3 modes
//   - basic input/output
//   - strobed input/output
//   - bidirectional
//  GroupB can only use the first two modes.
//  Only the first mode is used on MSX, only this mode is implemented yet.
//
//  for more detail see
//    http://w3.qahwah.net/joost/openMSX/8255.pdf

#ifndef MSXPPI_HH
#define MSXPPI_HH

#include "MSXDevice.hh"
#include "I8255Interface.hh"
#include <memory>

namespace openmsx {

class I8255;
class KeyClick;
class CassettePortInterface;
class RenShaTurbo;
class Keyboard;

class MSXPPI: public MSXDevice, public I8255Interface
{
public:
	explicit MSXPPI(const DeviceConfig& config);
	virtual ~MSXPPI();

	virtual void reset(EmuTime::param time);
	virtual void powerDown(EmuTime::param time);
	virtual byte readIO(word port, EmuTime::param time);
	virtual byte peekIO(word port, EmuTime::param time) const;
	virtual void writeIO(word port, byte value, EmuTime::param time);

	template<typename Archive>
	void serialize(Archive& ar, unsigned version);

private:
	// I8255Interface
	virtual byte readA(EmuTime::param time);
	virtual byte readB(EmuTime::param time);
	virtual nibble readC0(EmuTime::param time);
	virtual nibble readC1(EmuTime::param time);
	virtual byte peekA(EmuTime::param time) const;
	virtual byte peekB(EmuTime::param time) const;
	virtual nibble peekC0(EmuTime::param time) const;
	virtual nibble peekC1(EmuTime::param time) const;
	virtual void writeA(byte value, EmuTime::param time);
	virtual void writeB(byte value, EmuTime::param time);
	virtual void writeC0(nibble value, EmuTime::param time);
	virtual void writeC1(nibble value, EmuTime::param time);

	CassettePortInterface& cassettePort;
	RenShaTurbo& renshaTurbo;
	const std::auto_ptr<I8255> i8255;
	const std::auto_ptr<KeyClick> click;
	const std::auto_ptr<Keyboard> keyboard;
	nibble prevBits;
	nibble selectedRow;
};

} // namespace openmsx

#endif
