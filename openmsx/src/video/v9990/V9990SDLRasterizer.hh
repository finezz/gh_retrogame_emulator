// $Id: V9990SDLRasterizer.hh 12695 2012-07-06 09:21:11Z m9710797 $

#ifndef V9990SDLRASTERIZER_HH
#define V9990SDLRASTERIZER_HH

#include "V9990Rasterizer.hh"
#include "Observer.hh"
#include "noncopyable.hh"
#include <memory>

namespace openmsx {

class Display;
class V9990;
class V9990VRAM;
class RawFrame;
class OutputSurface;
class VisibleSurface;
class RenderSettings;
class Setting;
class PostProcessor;
template <class Pixel> class V9990BitmapConverter;
template <class Pixel> class V9990P1Converter;
template <class Pixel> class V9990P2Converter;

/** Rasterizer using SDL.
  */
template <class Pixel>
class V9990SDLRasterizer : public V9990Rasterizer, private noncopyable,
                           private Observer<Setting>
{
public:
	V9990SDLRasterizer(
		V9990& vdp, Display& display, VisibleSurface& screen,
		std::auto_ptr<PostProcessor> postProcessor);
	virtual ~V9990SDLRasterizer();

	// Rasterizer interface:
	virtual bool isActive();
	virtual void reset();
	virtual void frameStart();
	virtual void frameEnd(EmuTime::param time);
	virtual void setDisplayMode(V9990DisplayMode displayMode);
	virtual void setColorMode(V9990ColorMode colorMode);
	virtual void setPalette(int index, byte r, byte g, byte b, bool ys);
	virtual void setSuperimpose(bool enabled);
	virtual void drawBorder(int fromX, int fromY, int limitX, int limitY);
	virtual void drawDisplay(int fromX, int fromY, int toX, int toY,
	                         int displayX,
	                         int displayY, int displayYA, int displayYB);
	virtual bool isRecording() const;

private:
	/** screen width for SDLLo
	  */
	static const int SCREEN_WIDTH  = 320;

	/** screenheight for SDLLo
	  */
	static const int SCREEN_HEIGHT = 240;

	/** The VDP of which the video output is being rendered.
	  */
	V9990& vdp;

	/** The VRAM whose contents are rendered.
	  */
	V9990VRAM& vram;

	/** The surface which is visible to the user.
	  */
	OutputSurface& screen;

	/** The next frame as it is delivered by the VDP, work in progress.
	  */
	std::auto_ptr<RawFrame> workFrame;

	/** The current renderer settings (gamma, brightness, contrast)
	  */
	RenderSettings& renderSettings;

	/** Line to render at top of display.
	  * After all, our screen is 240 lines while display is 262 or 313.
	  */
	int lineRenderTop;

	/** First display column to draw.  Since the width of the VDP lines <=
	  * the screen width, colZero is <= 0. The non-displaying parts of the
	  * screen will be filled as border.
	  */
	int colZero;

	/** The current screen mode
	  */
	V9990DisplayMode displayMode;
	V9990ColorMode   colorMode;

	/** Palette containing the complete V9990 Color space
	  */
	Pixel palette32768[32768];

	/** The 256 color palette. A fixed subset of the palette32768.
	  */
	Pixel palette256[256];

	/** The 64 palette entries of the VDP - a subset of the palette32768.
	  * These are colors influenced by the palette IO ports and registers
	  */
	Pixel palette64[64];

	/** The video post processor which displays the frames produced by this
	  *  rasterizer.
	  */
	const std::auto_ptr<PostProcessor> postProcessor;

	/** Bitmap converter. Converts VRAM into pixels
	  */
	const std::auto_ptr<V9990BitmapConverter<Pixel> > bitmapConverter;

	/** P1 Converter
	  */
	const std::auto_ptr<V9990P1Converter<Pixel> > p1Converter;

	/** P2 Converter
	  */
	const std::auto_ptr<V9990P2Converter<Pixel> > p2Converter;

	/** Fill the palettes.
	  */
	void preCalcPalettes();
	void resetPalette();

	void drawP1Mode(int fromX, int fromY, int displayX,
	                int displayY, int displayYA, int displayYB,
	                int displayWidth, int displayHeight, bool drawSprites);
	void drawP2Mode(int fromX, int fromY, int displayX,
	                int displayY, int displayYA,
	                int displayWidth, int displayHeight, bool drawSprites);
	void drawBxMode(int fromX, int fromY, int displayX,
	                int displayY, int displayYA,
	                int displayWidth, int displayHeight, bool drawSprites);

	// Observer<Setting>
	virtual void update(const Setting& setting);
};

} // namespace openmsx

#endif
