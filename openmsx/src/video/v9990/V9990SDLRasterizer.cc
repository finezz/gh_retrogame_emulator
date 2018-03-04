// $Id: V9990SDLRasterizer.cc 12699 2012-07-07 12:46:06Z m9710797 $

#include "V9990SDLRasterizer.hh"
#include "V9990.hh"
#include "RawFrame.hh"
#include "PostProcessor.hh"
#include "V9990BitmapConverter.hh"
#include "V9990P1Converter.hh"
#include "V9990P2Converter.hh"
#include "BooleanSetting.hh"
#include "FloatSetting.hh"
#include "StringSetting.hh"
#include "MSXMotherBoard.hh"
#include "Display.hh"
#include "VisibleSurface.hh"
#include "RenderSettings.hh"
#include "MemoryOps.hh"
#include "build-info.hh"
#include "components.hh"
#include <algorithm>

using std::min;
using std::max;

namespace openmsx {

template <class Pixel>
V9990SDLRasterizer<Pixel>::V9990SDLRasterizer(
		V9990& vdp_, Display& display, VisibleSurface& screen_,
		std::auto_ptr<PostProcessor> postProcessor_
		)
	: vdp(vdp_), vram(vdp.getVRAM())
	, screen(screen_)
	, workFrame(new RawFrame(screen.getSDLFormat(), 1280, 240))
	, renderSettings(display.getRenderSettings())
	, displayMode(P1) // dummy value
	, colorMode(PP)   //   avoid UMR
	, postProcessor(postProcessor_)
	, bitmapConverter(new V9990BitmapConverter<Pixel>(
	                           vdp, palette64, palette256, palette32768))
	, p1Converter(new V9990P1Converter<Pixel>(vdp, palette64))
	, p2Converter(new V9990P2Converter<Pixel>(vdp, palette64))
{
	// Fill palettes
	preCalcPalettes();

	renderSettings.getGamma()      .attach(*this);
	renderSettings.getBrightness() .attach(*this);
	renderSettings.getContrast()   .attach(*this);
	renderSettings.getColorMatrix().attach(*this);
}

template <class Pixel>
V9990SDLRasterizer<Pixel>::~V9990SDLRasterizer()
{
	renderSettings.getColorMatrix().detach(*this);
	renderSettings.getGamma()      .detach(*this);
	renderSettings.getBrightness() .detach(*this);
	renderSettings.getContrast()   .detach(*this);
}

template <class Pixel>
bool V9990SDLRasterizer<Pixel>::isActive()
{
	return postProcessor->needRender() &&
	       vdp.getMotherBoard().isActive() &&
	       !vdp.getMotherBoard().isFastForwarding();
}

template <class Pixel>
void V9990SDLRasterizer<Pixel>::reset()
{
	setDisplayMode(vdp.getDisplayMode());
	setColorMode(vdp.getColorMode());
	resetPalette();
}

template <class Pixel>
void V9990SDLRasterizer<Pixel>::frameStart()
{
	const V9990DisplayPeriod& horTiming = vdp.getHorizontalTiming();
	const V9990DisplayPeriod& verTiming = vdp.getVerticalTiming();

	// Center image on the window.

	// In SDLLo, one window pixel represents 8 UC clockticks, so the
	// window = 320 * 8 UC ticks wide. In SDLHi, one pixel is 4 clock-
	// ticks and the window 640 pixels wide -- same amount of UC ticks.
	colZero = horTiming.blank + horTiming.border1 +
	          (horTiming.display - SCREEN_WIDTH * 8) / 2;

	// 240 display lines can be shown. In SDLHi, we can do interlace,
	// but still 240 lines per frame.
	lineRenderTop = verTiming.blank + verTiming.border1 +
	                (verTiming.display - SCREEN_HEIGHT) / 2;
}

template <class Pixel>
void V9990SDLRasterizer<Pixel>::frameEnd(EmuTime::param time)
{
	workFrame = postProcessor->rotateFrames(
	    workFrame,
	    vdp.isInterlaced() ? (vdp.getEvenOdd() ? RawFrame::FIELD_EVEN
	                                           : RawFrame::FIELD_ODD)
	                       : RawFrame::FIELD_NONINTERLACED,
	    time);
}

template <class Pixel>
void V9990SDLRasterizer<Pixel>::setDisplayMode(V9990DisplayMode mode)
{
	displayMode = mode;
	bitmapConverter->setColorMode(colorMode, displayMode);
}

template <class Pixel>
void V9990SDLRasterizer<Pixel>::setColorMode(V9990ColorMode mode)
{
	colorMode = mode;
	bitmapConverter->setColorMode(colorMode, displayMode);
}

template <class Pixel>
void V9990SDLRasterizer<Pixel>::drawBorder(
	int fromX, int fromY, int limitX, int limitY)
{
	Pixel bgColor = vdp.isOverScan()
	              ? 0
	              : palette64[vdp.getBackDropColor() & 63];

	int startY = max(fromY  - lineRenderTop,   0);
	int endY   = min(limitY - lineRenderTop, 240);
	if (startY >= endY) return;

	if ((fromX == 0) && (limitX == V9990DisplayTiming::UC_TICKS_PER_LINE)) {
		// optimization
		for (int y = startY; y < endY; ++y) {
			workFrame->setBlank(y, bgColor);
		}
		return;
	}

	static int const screenW = SCREEN_WIDTH * 8; // in ticks
	int startX = max(0, V9990::UCtoX(fromX - colZero, displayMode));
	int endX = V9990::UCtoX(
		(limitX == V9990DisplayTiming::UC_TICKS_PER_LINE)
		? screenW : min(screenW, limitX - colZero), displayMode);
	if (startX >= endX) return;

	unsigned lineWidth = vdp.getLineWidth();
	MemoryOps::MemSet<Pixel, MemoryOps::NO_STREAMING> memset;
	for (int y = startY; y < endY; ++y) {
		memset(workFrame->getLinePtrDirect<Pixel>(y) + startX,
		       endX - startX, bgColor);
		workFrame->setLineWidth(y, lineWidth);
	}
}

template <class Pixel>
void V9990SDLRasterizer<Pixel>::drawDisplay(
	int fromX, int fromY, int toX, int toY,
	int displayX, int displayY, int displayYA, int displayYB)
{
	static int const screenW = SCREEN_WIDTH * 8;
	static int const screenH = SCREEN_HEIGHT;

	// from VDP coordinates to screen coordinates
	fromX -= colZero;
	toX   -= colZero;
	fromY -= lineRenderTop;
	toY   -= lineRenderTop;

	// Clip to screen
	if (fromX < 0) {
		displayX -= fromX;
		fromX = 0;
	}
	if (fromY < 0) {
		displayY  -= fromY;
		displayYA -= fromY;
		displayYB -= fromY;
		fromY = 0;
	}
	if (toX > screenW) {
		toX = screenW;
	}
	if (toY > screenH) {
		toY = screenH;
	}
	fromX = V9990::UCtoX(fromX, displayMode);
	toX   = V9990::UCtoX(toX,   displayMode);

	if ((toX > fromX) && (toY > fromY)) {
		bool drawSprites = vdp.spritesEnabled() &&
			!renderSettings.getDisableSprites().getValue();

		displayX = V9990::UCtoX(displayX, displayMode);
		int displayWidth  = toX - fromX;
		int displayHeight = toY - fromY;

		if (displayMode == P1) {
			drawP1Mode(fromX, fromY, displayX,
			           displayY, displayYA, displayYB,
			           displayWidth, displayHeight,
			           drawSprites);
		} else if (displayMode == P2) {
			drawP2Mode(fromX, fromY, displayX,
			           displayY, displayYA,
			           displayWidth, displayHeight,
			           drawSprites);
		} else {
			drawBxMode(fromX, fromY, displayX,
			           displayY, displayYA,
			           displayWidth, displayHeight,
			           drawSprites);
		}
	}
}

template <class Pixel>
void V9990SDLRasterizer<Pixel>::drawP1Mode(
	int fromX, int fromY, int displayX,
	int displayY, int displayYA, int displayYB,
	int displayWidth, int displayHeight, bool drawSprites)
{
	while (displayHeight--) {
		Pixel* pixelPtr = workFrame->getLinePtrDirect<Pixel>(fromY) + fromX;
		p1Converter->convertLine(pixelPtr, displayX, displayWidth,
		                         displayY, displayYA, displayYB,
					 drawSprites);
		workFrame->setLineWidth(fromY, 320);
		++fromY;
		++displayY;
		++displayYA;
		++displayYB;
	}
}

template <class Pixel>
void V9990SDLRasterizer<Pixel>::drawP2Mode(
	int fromX, int fromY, int displayX, int displayY, int displayYA,
	int displayWidth, int displayHeight, bool drawSprites)
{
	while (displayHeight--) {
		Pixel* pixelPtr = workFrame->getLinePtrDirect<Pixel>(fromY) + fromX;
		p2Converter->convertLine(pixelPtr, displayX, displayWidth,
		                         displayY, displayYA, drawSprites);
		workFrame->setLineWidth(fromY, 640);
		++fromY;
		++displayY;
		++displayYA;
	}
}

template <class Pixel>
void V9990SDLRasterizer<Pixel>::drawBxMode(
	int fromX, int fromY, int displayX, int displayY, int displayYA,
	int displayWidth, int displayHeight, bool drawSprites)
{
	unsigned scrollX = vdp.getScrollAX();
	unsigned x = displayX + scrollX;

	int lineStep = 1;
	if (vdp.isEvenOddEnabled()) {
		if (vdp.getEvenOdd()) {
			++displayY;
			++displayYA;
		}
		lineStep = 2;
	}

	unsigned scrollY = vdp.getScrollAY();
	unsigned rollMask = vdp.getRollMask(0x1FFF);
	unsigned scrollYBase = scrollY & ~rollMask & 0x1FFF;
	int cursorY = displayY - vdp.getCursorYOffset();
	while (displayHeight--) {
		// Note: convertLine() can draw up to 3 pixels too many. But
		// that's ok, the buffer is big enough: buffer can hold 1280
		// pixels, max displayWidth is 1024 pixels. When taking the
		// position of the borders into account, the display area
		// plus 3 pixels cannot go beyond the end of the buffer.
		unsigned y = scrollYBase + ((displayYA + scrollY) & rollMask);
		Pixel* pixelPtr = workFrame->getLinePtrDirect<Pixel>(fromY) + fromX;
		bitmapConverter->convertLine(pixelPtr, x, y, displayWidth,
		                             cursorY, drawSprites);
		workFrame->setLineWidth(fromY, vdp.getLineWidth());
		++fromY;
		displayYA += lineStep;
		cursorY   += lineStep;
	}
}


template <class Pixel>
void V9990SDLRasterizer<Pixel>::preCalcPalettes()
{
	// the 32768 color palette
	for (int g = 0; g < 32; ++g) {
		for (int r = 0; r < 32; ++r) {
			for (int b = 0; b < 32; ++b) {
				double dr = r / 31.0;
				double dg = g / 31.0;
				double db = b / 31.0;
				renderSettings.transformRGB(dr, dg, db);
				palette32768[(g << 10) + (r << 5) + b] =
					screen.mapRGB(dr, dg, db);
			}
		}
	}

	// the 256 color palette
	int mapRG[8] = { 0, 4, 9, 13, 18, 22, 27, 31 };
	int mapB [4] = { 0, 11, 21, 31 };
	for (int g = 0; g < 8; ++g) {
		for (int r = 0; r < 8; ++r) {
			for (int b = 0; b < 4; ++b) {
				palette256[(g << 5) + (r << 2) + b] =
					palette32768[(mapRG[g] << 10) +
					             (mapRG[r] <<  5) +
					              mapB [b]];
			}
		}
	}

	resetPalette();
}

template <class Pixel>
void V9990SDLRasterizer<Pixel>::setPalette(int index,
                                           byte r, byte g, byte b, bool ys)
{
	palette64[index & 63] = ys ? screen.getKeyColor<Pixel>()
	                           : palette32768[((g & 31) << 10) +
	                                          ((r & 31) <<  5) +
	                                          ((b & 31) <<  0)];
}

template <class Pixel>
void V9990SDLRasterizer<Pixel>::resetPalette()
{
	// get 64 color palette from VDP
	for (int i = 0; i < 64; ++i) {
		byte r, g, b;
		bool ys;
		vdp.getPalette(i, r, g, b, ys);
		setPalette(i, r, g, b, ys);
	}
	palette256[0] = vdp.isSuperimposing() ? screen.getKeyColor<Pixel>()
	                                      : palette32768[0];
}

template <class Pixel>
void V9990SDLRasterizer<Pixel>::setSuperimpose(bool /*enabled*/)
{
	resetPalette();
}

template <class Pixel>
bool V9990SDLRasterizer<Pixel>::isRecording() const
{
	return postProcessor->isRecording();
}

template <class Pixel>
void V9990SDLRasterizer<Pixel>::update(const Setting& setting)
{
	if ((&setting == &renderSettings.getGamma()) ||
	    (&setting == &renderSettings.getBrightness()) ||
	    (&setting == &renderSettings.getContrast()) ||
	    (&setting == &renderSettings.getColorMatrix())) {
		preCalcPalettes();
	}
}

// Force template instantiation.
#if HAVE_16BPP
template class V9990SDLRasterizer<word>;
#endif
#if HAVE_32BPP || COMPONENT_GL
template class V9990SDLRasterizer<unsigned>;
#endif

} // namespace openmsx
