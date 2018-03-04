// $Id: ThrottleManager.cc 8194 2008-09-02 18:17:55Z m9710797 $

#include "ThrottleManager.hh"
#include "BooleanSetting.hh"

namespace openmsx {

// class ThrottleManager:

ThrottleManager::ThrottleManager(CommandController& commandController)
	: throttleSetting(new BooleanSetting(commandController, "throttle",
		"controls speed throttling", true, Setting::DONT_SAVE))
	, fullSpeedLoadingSetting(new BooleanSetting(commandController,
		"fullspeedwhenloading",
		"sets openMSX to full speed when the MSX is loading", false))
	, loading(0), throttle(true)
{
	throttleSetting->attach(*this);
	fullSpeedLoadingSetting->attach(*this);
}

ThrottleManager::~ThrottleManager()
{
	throttleSetting->detach(*this);
	fullSpeedLoadingSetting->detach(*this);
}

void ThrottleManager::updateStatus()
{
	bool newThrottle = throttleSetting->getValue() &&
	                   (!loading || !fullSpeedLoadingSetting->getValue());
	if (throttle != newThrottle) {
		throttle = newThrottle;
		notify();
	}
}

bool ThrottleManager::isThrottled() const
{
	return throttle;
}

void ThrottleManager::indicateLoadingState(bool state)
{
	if (state) {
		++loading;
	} else {
		--loading;
	}
	assert(loading >= 0);
	updateStatus();
}

void ThrottleManager::update(const Setting& /*setting*/)
{
	updateStatus();
}


// class LoadingIndicator:

LoadingIndicator::LoadingIndicator(ThrottleManager& throttleManager_)
	: throttleManager(throttleManager_)
	, isLoading(false)
{
}

LoadingIndicator::~LoadingIndicator()
{
	update(false);
}

void LoadingIndicator::update(bool newState)
{
	if (isLoading != newState) {
		isLoading = newState;
		throttleManager.indicateLoadingState(isLoading);
	}
}

} // namespace openmsx
