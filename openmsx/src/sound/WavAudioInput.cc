// $Id: WavAudioInput.cc 12631 2012-06-14 20:18:24Z m9710797 $

#include "WavAudioInput.hh"
#include "CommandController.hh"
#include "PlugException.hh"
#include "FilenameSetting.hh"
#include "CliComm.hh"
#include "WavData.hh"
#include "serialize.hh"

using std::string;

namespace openmsx {

WavAudioInput::WavAudioInput(CommandController& commandController)
	: audioInputFilenameSetting(new FilenameSetting(
		commandController, "audio-inputfilename",
		"filename of the file where the sampler reads data from",
		"audio-input.wav"))
	, reference(EmuTime::zero)
{
	audioInputFilenameSetting->attach(*this);
}

WavAudioInput::~WavAudioInput()
{
	audioInputFilenameSetting->detach(*this);
}

void WavAudioInput::loadWave()
{
	wav.reset(new WavData(audioInputFilenameSetting->getValue(), 16, 0));
}

const string& WavAudioInput::getName() const
{
	static const string name("wavinput");
	return name;
}

string_ref WavAudioInput::getDescription() const
{
	return "Read .wav files. Can for example be used as input for "
		"samplers.";
}

void WavAudioInput::plugHelper(Connector& /*connector*/, EmuTime::param time)
{
	if (!wav.get()) {
		try {
			loadWave();
		} catch (MSXException& e) {
			throw PlugException("Load of wave file failed: " +
			                    e.getMessage());
		}
	}
	reference = time;
}

void WavAudioInput::unplugHelper(EmuTime::param /*time*/)
{
	wav.reset();
}

void WavAudioInput::update(const Setting& setting)
{
	(void)setting;
	assert(&setting == audioInputFilenameSetting.get());
	if (isPluggedIn()) {
		try {
			loadWave();
		} catch (MSXException& e) {
			// TODO proper error handling, message should go to console
			setting.getCommandController().getCliComm().printWarning(
				"Load of wave file failed: " + e.getMessage());
		}
	}
}

short WavAudioInput::readSample(EmuTime::param time)
{
	if (wav.get()) {
		unsigned pos = (time - reference).getTicksAt(wav->getFreq());
		if (pos < wav->getSize()) {
			const short* buf =
				static_cast<const short*>(wav->getData());
			return buf[pos];
		}
	}
	return 0;
}

template<typename Archive>
void WavAudioInput::serialize(Archive& ar, unsigned /*version*/)
{
	ar.serialize("reference", reference);
	if (ar.isLoader()) {
		update(*audioInputFilenameSetting);
	}
}
INSTANTIATE_SERIALIZE_METHODS(WavAudioInput);
REGISTER_POLYMORPHIC_INITIALIZER(Pluggable, WavAudioInput, "WavAudioInput");

} // namespace openmsx
