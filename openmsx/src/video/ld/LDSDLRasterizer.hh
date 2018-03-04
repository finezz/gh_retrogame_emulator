// $Id: LDSDLRasterizer.hh 12022 2011-03-15 20:13:49Z m9710797 $

#ifndef LDSDLRASTERIZER_HH
#define LDSDLRASTERIZER_HH

#include "LDRasterizer.hh"
#include "noncopyable.hh"
#include <SDL.h>
#include <memory>

namespace openmsx {

class VisibleSurface;
class RawFrame;
class PostProcessor;

/** Rasterizer using a frame buffer approach: it writes pixels to a single
  * rectangular pixel buffer.
  */
template <class Pixel>
class LDSDLRasterizer : public LDRasterizer, private noncopyable
{
public:
	LDSDLRasterizer(
		VisibleSurface& screen,
		std::auto_ptr<PostProcessor> postProcessor);
	virtual ~LDSDLRasterizer();

	// Rasterizer interface:
	virtual void frameStart(EmuTime::param time);
	virtual void drawBlank(int r, int g, int b);
	virtual RawFrame* getRawFrame();

private:
	/** The video post processor which displays the frames produced by this
	  *  rasterizer.
	  */
	const std::auto_ptr<PostProcessor> postProcessor;

	/** The next frame as it is delivered by the VDP, work in progress.
	  */
	std::auto_ptr<RawFrame> workFrame;

	const SDL_PixelFormat pixelFormat;
};

} // namespace openmsx

#endif
