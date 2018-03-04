// $Id: DummyRenderer.cc 12658 2012-06-23 19:58:36Z m9710797 $

#include "DummyRenderer.hh"
#include "DisplayMode.hh"

namespace openmsx {

void DummyRenderer::reInit() {
}

void DummyRenderer::frameStart(EmuTime::param /*time*/) {
}

void DummyRenderer::frameEnd(EmuTime::param /*time*/) {
}

void DummyRenderer::updateTransparency(bool /*enabled*/, EmuTime::param /*time*/) {
}

void DummyRenderer::updateSuperimposing(const RawFrame* /*videoSource*/,
                                        EmuTime::param /*time*/) {
}

void DummyRenderer::updateForegroundColor(int /*color*/, EmuTime::param /*time*/) {
}

void DummyRenderer::updateBackgroundColor(int /*color*/, EmuTime::param /*time*/) {
}

void DummyRenderer::updateBlinkForegroundColor(int /*color*/, EmuTime::param /*time*/) {
}

void DummyRenderer::updateBlinkBackgroundColor(int /*color*/, EmuTime::param /*time*/) {
}

void DummyRenderer::updateBlinkState(bool /*enabled*/, EmuTime::param /*time*/) {
}

void DummyRenderer::updatePalette(int /*index*/, int /*grb*/, EmuTime::param /*time*/) {
}

void DummyRenderer::updateVerticalScroll(int /*scroll*/, EmuTime::param /*time*/) {
}

void DummyRenderer::updateHorizontalScrollLow(byte /*scroll*/, EmuTime::param /*time*/) {
}

void DummyRenderer::updateHorizontalScrollHigh(byte /*scroll*/, EmuTime::param /*time*/) {
}

void DummyRenderer::updateBorderMask(bool /*masked*/, EmuTime::param /*time*/) {
}

void DummyRenderer::updateMultiPage(bool /*multiPage*/, EmuTime::param /*time*/) {
}

void DummyRenderer::updateHorizontalAdjust(int /*adjust*/, EmuTime::param /*time*/) {
}

void DummyRenderer::updateDisplayEnabled(bool /*enabled*/, EmuTime::param /*time*/) {
}

void DummyRenderer::updateDisplayMode(DisplayMode /*mode*/, EmuTime::param /*time*/) {
}

void DummyRenderer::updateNameBase(int /*addr*/, EmuTime::param /*time*/) {
}

void DummyRenderer::updatePatternBase(int /*addr*/, EmuTime::param /*time*/) {
}

void DummyRenderer::updateColorBase(int /*addr*/, EmuTime::param /*time*/) {
}

void DummyRenderer::updateSpritesEnabled(bool /*enabled*/, EmuTime::param /*time*/) {
}

void DummyRenderer::updateVRAM(unsigned /*offset*/, EmuTime::param /*time*/) {
}

void DummyRenderer::updateWindow(bool /*enabled*/, EmuTime::param /*time*/) {
}

void DummyRenderer::paint(OutputSurface& /*output*/) {
}

string_ref DummyRenderer::getLayerName() const {
	return "DummyRenderer";
}

} // namespace openmsx
