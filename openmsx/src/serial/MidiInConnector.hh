// $Id: MidiInConnector.hh 12630 2012-06-14 20:17:01Z m9710797 $

#ifndef MIDIINCONNECTOR_HH
#define MIDIINCONNECTOR_HH

#include "Connector.hh"
#include "SerialDataInterface.hh"
#include "serialize_meta.hh"

namespace openmsx {

class MidiInDevice;

class MidiInConnector : public Connector, public SerialDataInterface
{
public:
	MidiInConnector(PluggingController& pluggingController,
	                string_ref name);
	virtual ~MidiInConnector();

	MidiInDevice& getPluggedMidiInDev() const;

	// Connector
	virtual const std::string getDescription() const;
	virtual string_ref getClass() const;

	virtual bool ready() = 0;
	virtual bool acceptsData() = 0;

	template<typename Archive>
	void serialize(Archive& ar, unsigned version);
};

REGISTER_BASE_CLASS(MidiInConnector, "inConnector");

} // namespace openmsx

#endif
