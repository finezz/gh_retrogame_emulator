// $Id: DummyMidiInDevice.hh 12631 2012-06-14 20:18:24Z m9710797 $

#ifndef DUMMYMIDIINDEVICE_HH
#define DUMMYMIDIINDEVICE_HH

#include "MidiInDevice.hh"

namespace openmsx {

class DummyMidiInDevice : public MidiInDevice
{
public:
	virtual void signal(EmuTime::param time);
	virtual string_ref getDescription() const;
	virtual void plugHelper(Connector& connector, EmuTime::param time);
	virtual void unplugHelper(EmuTime::param time);
};

} // namespace openmsx

#endif
