// $Id: MSXAudio.hh 12527 2012-05-17 17:34:11Z m9710797 $

#ifndef MSXAUDIO_HH
#define MSXAUDIO_HH

#include "MSXDevice.hh"
#include <memory>
#include <string>

namespace openmsx {

class Y8950;
class Y8950Periphery;
class DACSound8U;

class MSXAudio : public MSXDevice
{
public:
	explicit MSXAudio(const DeviceConfig& config);
	virtual ~MSXAudio();

	/** Creates a periphery object for this MSXAudio cartridge.
	  * The ownership of the object remains with the MSXAudio instance.
	  */
	Y8950Periphery& createPeriphery(const std::string& soundDeviceName);

	virtual void powerUp(EmuTime::param time);
	virtual void reset(EmuTime::param time);
	virtual byte readIO(word port, EmuTime::param time);
	virtual byte peekIO(word port, EmuTime::param time) const;
	virtual void writeIO(word port, byte value, EmuTime::param time);
	virtual byte readMem(word address, EmuTime::param time);
	virtual byte peekMem(word address, EmuTime::param time) const;
	virtual void writeMem(word address, byte value, EmuTime::param time);
	virtual const byte* getReadCacheLine(word start) const;
	virtual byte* getWriteCacheLine(word start) const;

	template<typename Archive>
	void serialize(Archive& ar, unsigned version);

private:
	void enableDAC(bool enable, EmuTime::param time);

	std::auto_ptr<Y8950Periphery> periphery;
	std::auto_ptr<Y8950> y8950;
	std::auto_ptr<DACSound8U> dac;
	int registerLatch;
	byte dacValue;
	bool dacEnabled;

	friend class MusicModulePeriphery;
	friend class PanasonicAudioPeriphery;
	friend class ToshibaAudioPeriphery;
};

} // namespace openmsx

#endif
