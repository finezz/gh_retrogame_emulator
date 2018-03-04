// $Id: SoundDriver.hh 12242 2011-08-27 14:59:06Z m9710797 $

#ifndef SOUNDDRIVER_HH
#define SOUNDDRIVER_HH

namespace openmsx {

class SoundDriver
{
public:
	virtual ~SoundDriver() {}

	/** Mute the sound system
	 */
	virtual void mute() = 0;

	/** Unmute the sound system
	 */
	virtual void unmute() = 0;

	/** Returns the actual sample frequency. This might be different
	  * from the requested frequency ('frequency' setting).
	  */
	virtual unsigned getFrequency() const = 0;

	/** Get the number of samples that should be created 'per fragment'.
	  * This is not the same value as the 'samples setting'.
	  */
	virtual unsigned getSamples() const = 0;

	virtual void uploadBuffer(short* buffer, unsigned len) = 0;

protected:
	SoundDriver() {}
};

} // namespace openmsx

#endif
