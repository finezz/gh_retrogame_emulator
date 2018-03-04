// $Id: HQ3xLiteScaler.hh 12123 2011-04-17 20:30:07Z m9710797 $

#ifndef HQ3XLITESCALER_HH
#define HQ3XLITESCALER_HH

#include "Scaler3.hh"
#include "PixelOperations.hh"

namespace openmsx {

template <class Pixel>
class HQ3xLiteScaler : public Scaler3<Pixel>
{
public:
	explicit HQ3xLiteScaler(const PixelOperations<Pixel>& pixelOps);

	virtual void scale2x1to9x3(FrameSource& src,
		unsigned srcStartY, unsigned srcEndY, unsigned srcWidth,
		ScalerOutput<Pixel>& dst, unsigned dstStartY, unsigned dstEndY);
	virtual void scale1x1to3x3(FrameSource& src,
		unsigned srcStartY, unsigned srcEndY, unsigned srcWidth,
		ScalerOutput<Pixel>& dst, unsigned dstStartY, unsigned dstEndY);
	virtual void scale4x1to9x3(FrameSource& src,
		unsigned srcStartY, unsigned srcEndY, unsigned srcWidth,
		ScalerOutput<Pixel>& dst, unsigned dstStartY, unsigned dstEndY);
	virtual void scale2x1to3x3(FrameSource& src,
		unsigned srcStartY, unsigned srcEndY, unsigned srcWidth,
		ScalerOutput<Pixel>& dst, unsigned dstStartY, unsigned dstEndY);
	virtual void scale8x1to9x3(FrameSource& src,
		unsigned srcStartY, unsigned srcEndY, unsigned srcWidth,
		ScalerOutput<Pixel>& dst, unsigned dstStartY, unsigned dstEndY);
	virtual void scale4x1to3x3(FrameSource& src,
		unsigned srcStartY, unsigned srcEndY, unsigned srcWidth,
		ScalerOutput<Pixel>& dst, unsigned dstStartY, unsigned dstEndY);

private:
	PixelOperations<Pixel> pixelOps;
};

} // namespace openmsx

#endif
