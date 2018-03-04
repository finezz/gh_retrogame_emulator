// $Id: MidiInConnector.cc 12630 2012-06-14 20:17:01Z m9710797 $

#include "MidiInConnector.hh"
#include "MidiInDevice.hh"
#include "DummyMidiInDevice.hh"
#include "checked_cast.hh"
#include "serialize.hh"

namespace openmsx {

MidiInConnector::MidiInConnector(PluggingController& pluggingController,
                                 string_ref name)
	: Connector(pluggingController, name,
	            std::auto_ptr<Pluggable>(new DummyMidiInDevice()))
{
}

MidiInConnector::~MidiInConnector()
{
}

const std::string MidiInConnector::getDescription() const
{
	return "MIDI-in connector";
}

string_ref MidiInConnector::getClass() const
{
	return "midi in";
}

MidiInDevice& MidiInConnector::getPluggedMidiInDev() const
{
	return *checked_cast<MidiInDevice*>(&getPlugged());
}

template<typename Archive>
void MidiInConnector::serialize(Archive& ar, unsigned /*version*/)
{
	ar.template serializeBase<Connector>(*this);
}
INSTANTIATE_SERIALIZE_METHODS(MidiInConnector);

} // namespace openmsx

