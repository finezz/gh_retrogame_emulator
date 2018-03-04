// $Id: MSXTurboRPause.cc 12528 2012-05-17 17:36:10Z m9710797 $

#include "MSXTurboRPause.hh"
#include "LedStatus.hh"
#include "MSXMotherBoard.hh"
#include "BooleanSetting.hh"
#include "serialize.hh"

namespace openmsx {

MSXTurboRPause::MSXTurboRPause(const DeviceConfig& config)
	: MSXDevice(config)
	, pauseSetting(new BooleanSetting(getCommandController(),
	               "turborpause", "status of the TurboR pause", false))
	, status(255)
	, pauseLed(false)
	, turboLed(false)
	, hwPause(false)
{
	pauseSetting->attach(*this);
	reset(EmuTime::dummy());
}

MSXTurboRPause::~MSXTurboRPause()
{
	powerDown(EmuTime::dummy());
	pauseSetting->detach(*this);
}

void MSXTurboRPause::powerDown(EmuTime::param dummy)
{
	writeIO(0, 0, dummy); // send LED OFF events (if needed)
}

void MSXTurboRPause::reset(EmuTime::param dummy)
{
	pauseSetting->changeValue(false);
	writeIO(0, 0, dummy);
}

byte MSXTurboRPause::readIO(word port, EmuTime::param time)
{
	return peekIO(port, time);
}

byte MSXTurboRPause::peekIO(word /*port*/, EmuTime::param /*time*/) const
{
	return pauseSetting->getValue() ? 1 : 0;
}

void MSXTurboRPause::writeIO(word /*port*/, byte value, EmuTime::param /*time*/)
{
	status = value;
	bool newTurboLed = (status & 0x80) != 0;
	if (newTurboLed != turboLed) {
		turboLed = newTurboLed;
		getLedStatus().setLed(LedStatus::TURBO, turboLed);
	}
	updatePause();
}

void MSXTurboRPause::update(const Setting& /*setting*/)
{
	updatePause();
}

void MSXTurboRPause::updatePause()
{
	bool newHwPause = (status & 0x02) && pauseSetting->getValue();
	if (newHwPause != hwPause) {
		hwPause = newHwPause;
		if (hwPause) {
			getMotherBoard().pause();
		} else {
			getMotherBoard().unpause();
		}
	}

	bool newPauseLed = (status & 0x01) || hwPause;
	if (newPauseLed != pauseLed) {
		pauseLed = newPauseLed;
		getLedStatus().setLed(LedStatus::PAUSE, pauseLed);
	}
}

template<typename Archive>
void MSXTurboRPause::serialize(Archive& ar, unsigned /*version*/)
{
	ar.template serializeBase<MSXDevice>(*this);
	ar.serialize("status", status);
	if (ar.isLoader()) {
		writeIO(0, status, EmuTime::dummy());
	}
}
INSTANTIATE_SERIALIZE_METHODS(MSXTurboRPause);
REGISTER_MSXDEVICE(MSXTurboRPause, "TurboRPause");

} // namespace openmsx
