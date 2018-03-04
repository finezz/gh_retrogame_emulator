// $Id: LDPixelRenderer.hh 10849 2009-11-30 15:47:53Z m9710797 $

#ifndef LDPIXELRENDERER_HH
#define LDPIXELRENDERER_HH

#include "LDRenderer.hh"
#include "noncopyable.hh"
#include <memory>

namespace openmsx {

class MSXMotherBoard;
class EventDistributor;
class Display;
class LDRasterizer;
class LaserdiscPlayer;

/** Generic implementation of a pixel-based Renderer.
  * Uses a Rasterizer to plot actual pixels for a specific video system.
  */
class LDPixelRenderer : public LDRenderer, private noncopyable
{
public:
	LDPixelRenderer(LaserdiscPlayer& ld, Display& display);
	virtual ~LDPixelRenderer();

	// Renderer interface:
	virtual void frameStart(EmuTime::param time);
	virtual void frameEnd();
	virtual void drawBlank(int r, int g, int b);
	virtual RawFrame* getRawFrame();

private:
	bool isActive() const;

	MSXMotherBoard& motherboard;
	EventDistributor& eventDistributor;
	const std::auto_ptr<LDRasterizer> rasterizer;
};

} // namespace openmsx

#endif
