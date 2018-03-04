// $Id: LedStatus.cc 12508 2012-05-12 18:24:39Z m9710797 $

#include "LedStatus.hh"
#include "Reactor.hh"
#include "AlarmEvent.hh"
#include "MSXCliComm.hh"
#include "BooleanSetting.hh"
#include "ReadOnlySetting.hh"
#include "Timer.hh"

namespace openmsx {

static std::string getLedName(LedStatus::Led led)
{
	static const char* const names[LedStatus::NUM_LEDS] = {
		"power", "caps", "kana", "pause", "turbo", "FDD"
	};
	return names[led];
}

LedStatus::LedStatus(
		EventDistributor& eventDistributor,
		CommandController& commandController,
		MSXCliComm& msxCliComm_)
	: msxCliComm(msxCliComm_)
	, alarm(new AlarmEvent(eventDistributor, *this, OPENMSX_THROTTLE_LED_EVENT))
{
	lastTime = Timer::getTime();
	for (int i = 0; i < NUM_LEDS; ++i) {
		ledValue[i] = false;
		std::string name = getLedName(static_cast<Led>(i));
		ledStatus[i].reset(new ReadOnlySetting<BooleanSetting>(
			commandController,
			"led_" + name,
			"Current status for LED: " + name,
			ledValue[i]));
	}
}

LedStatus::~LedStatus()
{
}

void LedStatus::setLed(Led led, bool status)
{
	if (ledValue[led] == status) return;
	ledValue[led] = status;

	// Some MSX programs generate tons of LED events (e.g. New Era uses
	// the LEDs as a VU meter while playing samples). Without throttling
	// all these events overload the host CPU. That's why we limit it to
	// 100 events per second.
	unsigned long long now = Timer::getTime();
	unsigned long long diff = now - lastTime;
	if (diff > 10000) { // 1/100 s
		// handle now
		lastTime = now;
		handleEvent(led);
	} else {
		// schedule to handle it later, if we didn't plan to do so already
		if (!alarm->pending()) {
			alarm->schedule(10000 - diff);
		}
	}
}

void LedStatus::handleEvent(Led led)
{
	ledStatus[led]->setReadOnlyValue(ledValue[led]);

	static const std::string ON  = "on";
	static const std::string OFF = "off";
	msxCliComm.update(
		CliComm::LED, getLedName(led),
		ledValue[led] ? ON : OFF);
}

int LedStatus::signalEvent(const shared_ptr<const Event>& /*event*/)
{
	// Runs in main thread.
	for (int i = 0; i < NUM_LEDS; ++i) {
		if (ledValue[i] != ledStatus[i]->getValue()) {
			handleEvent(static_cast<Led>(i));
		}
	}
	lastTime = Timer::getTime();
	return 0;
}

} // namespace openmsx
