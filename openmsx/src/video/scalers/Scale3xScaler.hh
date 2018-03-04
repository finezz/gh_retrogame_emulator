// $Id: Scale3xScaler.hh 12123 2011-04-17 20:30:07Z m9710797 $

#ifndef SCALE3XSCALER_HH
#define SCALE3XSCALER_HH

#include "Scaler3.hh"

namespace openmsx {

/** Runs the Scale3x scaler algorithm.
  */
template <class Pixel>
class Scale3xScaler : public Scaler3<Pixel>
{
public:
	explicit Scale3xScaler(const PixelOperations<Pixel>& pixelOps);

	virtual void scale1x1to3x3(FrameSource& src,
		unsigned srcStartY, unsigned srcEndY, unsigned srcWidth,
		ScalerOutput<Pixel>& dst, unsigned dstStartY, unsigned dstEndY);

private:
	void scaleLine1on3Half(Pixel* dst,
		const Pixel* src0, const Pixel* src1, const Pixel* src2,
		unsigned srcWidth);
	void scaleLine1on3Mid (Pixel* dst,
		const Pixel* src0, const Pixel* src1, const Pixel* src2,
		unsigned srcWidth);
};

} // namespace openmsx

#endif
