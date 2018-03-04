// $Id: DoubledFrame.cc 12833 2012-08-25 08:29:41Z m9710797 $

#include "DoubledFrame.hh"

namespace openmsx {

DoubledFrame::DoubledFrame(const SDL_PixelFormat& format)
	: FrameSource(format)
{
}

void DoubledFrame::init(FrameSource* field_, unsigned skip_)
{
	FrameSource::init(FIELD_NONINTERLACED);
	field = field_;
	skip = skip_;
	setHeight(2 * field->getHeight());
}

unsigned DoubledFrame::getLineWidth(unsigned line) const
{
	int t = line - skip;
	return (t >= 0) ? field->getLineWidth(t / 2) : 1;
}

const void* DoubledFrame::getLineInfo(unsigned line, unsigned& width) const
{
	static const unsigned blackPixel = 0; // both 16bppp and 32bpp
	int t = line - skip;
	if (t >= 0) {
		return field->getLineInfo(t / 2, width);
	} else {
		width = 1;
		return &blackPixel;
	}
}

} // namespace openmsx
