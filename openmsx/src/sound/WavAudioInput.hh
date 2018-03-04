// $Id: WavAudioInput.hh 12631 2012-06-14 20:18:24Z m9710797 $

#ifndef WAVAUDIOINPUT_HH
#define WAVAUDIOINPUT_HH

#include "AudioInputDevice.hh"
#include "Observer.hh"
#include "EmuTime.hh"
#include "serialize_meta.hh"
#include <memory>

namespace openmsx {

class CommandController;
class FilenameSetting;
class Setting;
class WavData;

class WavAudioInput : public AudioInputDevice, private Observer<Setting>
{
public:
	explicit WavAudioInput(CommandController& commandController);
	virtual ~WavAudioInput();

	// AudioInputDevice
	virtual const std::string& getName() const;
	virtual string_ref getDescription() const;
	virtual void plugHelper(Connector& connector, EmuTime::param time);
	virtual void unplugHelper(EmuTime::param time);
	virtual short readSample(EmuTime::param time);

	template<typename Archive>
	void serialize(Archive& ar, unsigned version);

private:
	void loadWave();
	void update(const Setting& setting);

	const std::auto_ptr<FilenameSetting> audioInputFilenameSetting;

	std::auto_ptr<WavData> wav;
	EmuTime reference;
};

} // namespace openmsx

#endif
