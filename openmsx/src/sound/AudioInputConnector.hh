// $Id: AudioInputConnector.hh 12630 2012-06-14 20:17:01Z m9710797 $

#ifndef AUDIOINPUTCONNECTOR_HH
#define AUDIOINPUTCONNECTOR_HH

#include "Connector.hh"

namespace openmsx {

class AudioInputDevice;

class AudioInputConnector : public Connector
{
public:
	AudioInputConnector(PluggingController& pluggingController,
	                    string_ref name);
	virtual ~AudioInputConnector();

	AudioInputDevice& getPluggedAudioDev() const;

	// Connector
	virtual const std::string getDescription() const;
	virtual string_ref getClass() const;

	short readSample(EmuTime::param time) const;

	template<typename Archive>
	void serialize(Archive& ar, unsigned version);
};

} // namespace openmsx

#endif
