// $Id: DeinterlacedFrame.hh 12833 2012-08-25 08:29:41Z m9710797 $

#ifndef DEINTERLACEDFRAME_HH
#define DEINTERLACEDFRAME_HH

#include "FrameSource.hh"

namespace openmsx {

/** Produces a deinterlaced video frame based on two other FrameSources
  * (typically two RawFrames) containing the even and odd field.
  * This class does not copy the data from the input FrameSources.
  */
class DeinterlacedFrame : public FrameSource
{
public:
	explicit DeinterlacedFrame(const SDL_PixelFormat& format);
	void init(FrameSource* evenField, FrameSource* oddField);

private:
	virtual unsigned getLineWidth(unsigned line) const;
	virtual const void* getLineInfo(unsigned linei, unsigned& width) const;

	/** The original frames whose data will be deinterlaced.
	  * The even frame is at index 0, the odd frame at index 1.
	  */
	FrameSource* fields[2];
};

} // namespace openmsx

#endif // DEINTERLACEDFRAME_HH
