// $Id: VideoLayer.cc 12699 2012-07-07 12:46:06Z m9710797 $

#include "VideoLayer.hh"
#include "RenderSettings.hh"
#include "Display.hh"
#include "Reactor.hh"
#include "GlobalSettings.hh"
#include "BooleanSetting.hh"
#include "VideoSourceSetting.hh"
#include "MSXEventDistributor.hh"
#include "MSXMotherBoard.hh"
#include "Event.hh"
#include "openmsx.hh"
#include <cassert>

namespace openmsx {

VideoLayer::VideoLayer(MSXMotherBoard& motherBoard_,
                       VideoSource videoSource_)
	: motherBoard(motherBoard_)
	, display(motherBoard.getReactor().getDisplay())
	, renderSettings(display.getRenderSettings())
	, videoSourceSetting(renderSettings.getVideoSource())
	, videoSourceActivator(new VideoSourceActivator(
              videoSourceSetting, videoSource_))
	, powerSetting(motherBoard.getReactor().getGlobalSettings().getPowerSetting())
	, videoSource(videoSource_)
	, activeVideo9000(INACTIVE)
{
	calcCoverage();
	calcZ();
	display.addLayer(*this);

	videoSourceSetting.attach(*this);
	powerSetting.attach(*this);
	motherBoard.getMSXEventDistributor().registerEventListener(*this);
}

VideoLayer::~VideoLayer()
{
	PRT_DEBUG("Destructing VideoLayer...");
	motherBoard.getMSXEventDistributor().unregisterEventListener(*this);
	powerSetting.detach(*this);
	videoSourceSetting.detach(*this);

	display.removeLayer(*this);
	PRT_DEBUG("Destructing VideoLayer... DONE!");
}

VideoSource VideoLayer::getVideoSource() const
{
	return videoSource;
}

void VideoLayer::update(const Setting& setting)
{
	if (&setting == &videoSourceSetting) {
		calcZ();
	} else if (&setting == &powerSetting) {
		calcCoverage();
	}
}

void VideoLayer::calcZ()
{
	setZ((renderSettings.getVideoSource().getValue() == videoSource)
		? Z_MSX_ACTIVE
		: Z_MSX_PASSIVE);
}

void VideoLayer::calcCoverage()
{
	Coverage coverage;

	if (!powerSetting.getValue() || !motherBoard.isActive()) {
		coverage = COVER_NONE;
	} else {
		coverage = COVER_FULL;
	}

	setCoverage(coverage);
}

void VideoLayer::signalEvent(const shared_ptr<const Event>& event,
                             EmuTime::param /*time*/)
{
	if ((event->getType() == OPENMSX_MACHINE_ACTIVATED) ||
	    (event->getType() == OPENMSX_MACHINE_DEACTIVATED)) {
		calcCoverage();
	}
}

bool VideoLayer::needRender() const
{
	VideoSource current = renderSettings.getVideoSource().getValue();
	return (current == videoSource) ||
	      ((current == VIDEO_9000) && (activeVideo9000 != INACTIVE));
}

bool VideoLayer::needRecord() const
{
	VideoSource current = renderSettings.getVideoSource().getValue();
	return (current == videoSource) ||
	      ((current == VIDEO_9000) && (activeVideo9000 == ACTIVE_FRONT));
}

} // namespace openmsx
