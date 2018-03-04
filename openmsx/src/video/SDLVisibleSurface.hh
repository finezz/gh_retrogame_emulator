// $Id: SDLVisibleSurface.hh 10135 2009-07-02 09:03:22Z mthuurne $

#ifndef SDLVISIBLESURFACE_HH
#define SDLVISIBLESURFACE_HH

#include "VisibleSurface.hh"

namespace openmsx {

class SDLVisibleSurface : public VisibleSurface
{
public:
	SDLVisibleSurface(unsigned width, unsigned height, bool fullscreen,
	                  RenderSettings& renderSettings,
	                  EventDistributor& eventDistributor,
	                  InputEventGenerator& inputEventGenerator);

private:
	// OutputSurface
	virtual void saveScreenshot(const std::string& filename);

	// VisibleSurface
	virtual void finish();
	virtual std::auto_ptr<Layer> createSnowLayer(Display& display);
	virtual std::auto_ptr<Layer> createConsoleLayer(
		Reactor& reactor, CommandConsole& console);
	virtual std::auto_ptr<Layer> createOSDGUILayer(OSDGUI& gui);
	virtual std::auto_ptr<OutputSurface> createOffScreenSurface();
};

} // namespace openmsx

#endif
