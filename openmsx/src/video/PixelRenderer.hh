// $Id: PixelRenderer.hh 10786 2009-11-16 15:28:16Z m9710797 $

#ifndef PIXELRENDERER_HH
#define PIXELRENDERER_HH

#include "Renderer.hh"
#include "Observer.hh"
#include "RenderSettings.hh"
#include "DisplayMode.hh"
#include "openmsx.hh"
#include "noncopyable.hh"
#include <memory>

namespace openmsx {

class EventDistributor;
class RealTime;
class Display;
class Rasterizer;
class VDP;
class VDPVRAM;
class SpriteChecker;
class Setting;

/** Generic implementation of a pixel-based Renderer.
  * Uses a Rasterizer to plot actual pixels for a specific video system.
  */
class PixelRenderer : public Renderer, private Observer<Setting>,
                      private noncopyable
{
public:
	PixelRenderer(VDP& vdp, Display& display);
	virtual ~PixelRenderer();

	// Renderer interface:
	virtual void reInit();
	virtual void frameStart(EmuTime::param time);
	virtual void frameEnd(EmuTime::param time);
	virtual void updateHorizontalScrollLow(byte scroll, EmuTime::param time);
	virtual void updateHorizontalScrollHigh(byte scroll, EmuTime::param time);
	virtual void updateBorderMask(bool masked, EmuTime::param time);
	virtual void updateMultiPage(bool multiPage, EmuTime::param time);
	virtual void updateTransparency(bool enabled, EmuTime::param time);
	virtual void updateSuperimposing(const RawFrame* videoSource, EmuTime::param time);
	virtual void updateForegroundColor(int color, EmuTime::param time);
	virtual void updateBackgroundColor(int color, EmuTime::param time);
	virtual void updateBlinkForegroundColor(int color, EmuTime::param time);
	virtual void updateBlinkBackgroundColor(int color, EmuTime::param time);
	virtual void updateBlinkState(bool enabled, EmuTime::param time);
	virtual void updatePalette(int index, int grb, EmuTime::param time);
	virtual void updateVerticalScroll(int scroll, EmuTime::param time);
	virtual void updateHorizontalAdjust(int adjust, EmuTime::param time);
	virtual void updateDisplayEnabled(bool enabled, EmuTime::param time);
	virtual void updateDisplayMode(DisplayMode mode, EmuTime::param time);
	virtual void updateNameBase(int addr, EmuTime::param time);
	virtual void updatePatternBase(int addr, EmuTime::param time);
	virtual void updateColorBase(int addr, EmuTime::param time);
	virtual void updateSpritesEnabled(bool enabled, EmuTime::param time);
	virtual void updateVRAM(unsigned offset, EmuTime::param time);
	virtual void updateWindow(bool enabled, EmuTime::param time);

private:
	/** Indicates whether the area to be drawn is border or display. */
	enum DrawType { DRAW_BORDER, DRAW_DISPLAY };

	// Observer<Setting> interface:
	virtual void update(const Setting& setting);

	/** Call the right draw method in the subclass,
	  * depending on passed drawType.
	  */
	void draw(
		int startX, int startY, int endX, int endY, DrawType drawType,
		bool atEnd);

	/** Subdivide an area specified by two scan positions into a series of
	  * rectangles.
	  * Clips the rectangles to { (x,y) | clipL <= x < clipR }.
	  * @param drawType
	  *   If DRAW_BORDER, draw rectangles using drawBorder;
	  *   if DRAW_DISPLAY, draw rectangles using drawDisplay.
	  */
	void subdivide(
		int startX, int startY, int endX, int endY,
		int clipL, int clipR, DrawType drawType );

	inline bool checkSync(int offset, EmuTime::param time);

	/** Update renderer state to specified moment in time.
	  * @param time Moment in emulated time to update to.
	  * @param force When screen accuracy is used,
	  *     rendering is only performed if this parameter is true.
	  */
	void sync(EmuTime::param time, bool force = false);

	/** Render lines until specified moment in time.
	  * Unlike sync(), this method does not sync with VDPVRAM.
	  * The VRAM should be to be up to date and remain unchanged
	  * from the current time to the specified time.
	  * @param time Moment in emulated time to render lines until.
	  */
	void renderUntil(EmuTime::param time);

	/** The VDP of which the video output is being rendered.
	  */
	VDP& vdp;

	/** The VRAM whose contents are rendered.
	  */
	VDPVRAM& vram;

	EventDistributor& eventDistributor;
	RealTime& realTime;
	RenderSettings& renderSettings;

	/** The sprite checker whose sprites are rendered.
	  */
	SpriteChecker& spriteChecker;

	const std::auto_ptr<Rasterizer> rasterizer;

	double finishFrameDuration;
	int frameSkipCounter;

	/** Number of the next position within a line to render.
	  * Expressed in VDP clock ticks since start of line.
	  */
	int nextX;

	/** Number of the next line to render.
	  * Expressed in number of lines since start of frame.
	  */
	int nextY;

	// internal VDP counter, actually belongs in VDP
	int textModeCounter;

	/** Accuracy setting for current frame.
	  */
	RenderSettings::Accuracy accuracy;

	/** Is display enabled?
	  * Enabled means the current line is in the display area and
	  * forced blanking is off.
	  */
	bool displayEnabled;

	/** Should current frame be draw or can it be skipped.
	  */
	bool renderFrame;
	bool prevRenderFrame;
};

} // namespace openmsx

#endif
