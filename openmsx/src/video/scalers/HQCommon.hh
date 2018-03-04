// $Id: HQCommon.hh 12123 2011-04-17 20:30:07Z m9710797 $

#ifndef HQCOMMON_HH
#define HQCOMMON_HH

#include "FrameSource.hh"
#include "ScalerOutput.hh"
#include "LineScalers.hh"
#include "PixelOperations.hh"
#include "build-info.hh"
#include <cassert>

namespace openmsx {

template <typename Pixel>
static inline unsigned readPixel(Pixel p)
{
	// TODO: Use surface info instead.
	if (sizeof(Pixel) == 2) {
		return ((p & 0xF800) << 8) |
		       ((p & 0x07C0) << 5) | // drop lowest green bit
		       ((p & 0x001F) << 3);
	} else {
		return p & 0xF8F8F8F8;
	}
}

template <typename Pixel>
static inline Pixel writePixel(unsigned p)
{
	// TODO: Use surface info instead.
	if (sizeof(Pixel) == 2) {
		return ((p & 0xF80000) >> 8) |
		       ((p & 0x00FC00) >> 5) |
		       ((p & 0x0000F8) >> 3);
	} else {
		return (p & 0xF8F8F8F8) | ((p & 0xE0E0E0E0) >> 5);
	}
}

class EdgeHQ
{
public:
	EdgeHQ(unsigned shiftR_, unsigned shiftG_, unsigned shiftB_)
		: shiftR(shiftR_), shiftG(shiftG_), shiftB(shiftB_)
	{
	}

	inline bool operator()(unsigned c1, unsigned c2) const
	{
		if (c1 == c2) return false;

		unsigned r1 = (c1 >> shiftR) & 0xFF;
		unsigned g1 = (c1 >> shiftG) & 0xFF;
		unsigned b1 = (c1 >> shiftB) & 0xFF;

		unsigned r2 = (c2 >> shiftR) & 0xFF;
		unsigned g2 = (c2 >> shiftG) & 0xFF;
		unsigned b2 = (c2 >> shiftB) & 0xFF;

		int dr = r1 - r2;
		int dg = g1 - g2;
		int db = b1 - b2;

		int dy = dr + dg + db;
		if (dy < -0xC0 || dy > 0xC0) return true;

		int du = dr - db;
		if (du < -0x1C || du > 0x1C) return true;

		int dv = 3 * dg - dy;
		if (dv < -0x30 || dv > 0x30) return true;

		return false;
	}
private:
	const unsigned shiftR;
	const unsigned shiftG;
	const unsigned shiftB;
};

template<typename Pixel>
EdgeHQ createEdgeHQ(const PixelOperations<Pixel>& pixelOps)
{
	if (sizeof(Pixel) == 2) {
		return EdgeHQ(0, 8, 16);
	} else {
		return EdgeHQ(pixelOps.getRshift(),
		              pixelOps.getGshift(),
		              pixelOps.getBshift());
	}
}

struct EdgeHQLite
{
	inline bool operator()(unsigned c1, unsigned c2) const
	{
		return c1 != c2;
	}
};

template <typename EdgeOp>
void calcEdgesGL(const unsigned* __restrict curr, const unsigned* __restrict next,
                 unsigned* __restrict edges2, EdgeOp edgeOp)
{
	typedef unsigned Pixel;
	if (OPENMSX_BIGENDIAN) {
		unsigned pattern = 0;
		Pixel c5 = curr[0];
		Pixel c8 = next[0];
		if (edgeOp(c5, c8)) pattern |= 0x1400;

		for (unsigned xx = 0; xx < (320 - 2) / 2; ++xx) {
			pattern = (pattern << (16 + 1)) & 0xA8000000;
			pattern |= ((edges2[xx] >> 5) & 0x01F001F0);

			if (edgeOp(c5, c8)) pattern |= 0x02000000;
			Pixel c6 = curr[2 * xx + 1];
			if (edgeOp(c6, c8)) pattern |= 0x10002000;
			if (edgeOp(c5, c6)) pattern |= 0x40008000;
			Pixel c9 = next[2 * xx + 1];
			if (edgeOp(c5, c9)) pattern |= 0x04000800;

			if (edgeOp(c6, c9)) pattern |= 0x0200;
			c5 = curr[2 * xx + 2];
			if (edgeOp(c5, c9)) pattern |= 0x1000;
			if (edgeOp(c6, c5)) pattern |= 0x4000;
			c8 = next[2 * xx + 2];
			if (edgeOp(c6, c8)) pattern |= 0x0400;

			edges2[xx] = pattern;
		}

		pattern = (pattern << (16 + 1)) & 0xA8000000;
		pattern |= ((edges2[159] >> 5) & 0x01F001F0);

		if (edgeOp(c5, c8)) pattern |= 0x02000000;
		Pixel c6 = curr[319];
		if (edgeOp(c6, c8)) pattern |= 0x10002000;
		if (edgeOp(c5, c6)) pattern |= 0x40008000;
		Pixel c9 = next[319];
		if (edgeOp(c5, c9)) pattern |= 0x04000800;

		if (edgeOp(c6, c9)) pattern |= 0x1600;

		edges2[159] = pattern;
	} else {
		unsigned pattern = 0;
		Pixel c5 = curr[0];
		Pixel c8 = next[0];
		if (edgeOp(c5, c8)) pattern |= 0x14000000;

		for (unsigned xx = 0; xx < (320 - 2) / 2; ++xx) {
			pattern = (pattern >> (16 -1)) & 0xA800;
			pattern |= ((edges2[xx] >> 5) & 0x01F001F0);

			if (edgeOp(c5, c8)) pattern |= 0x0200;
			Pixel c6 = curr[2 * xx + 1];
			if (edgeOp(c6, c8)) pattern |= 0x20001000;
			if (edgeOp(c5, c6)) pattern |= 0x80004000;
			Pixel c9 = next[2 * xx + 1];
			if (edgeOp(c5, c9)) pattern |= 0x08000400;

			if (edgeOp(c6, c9)) pattern |= 0x02000000;
			c5 = curr[2 * xx + 2];
			if (edgeOp(c5, c9)) pattern |= 0x10000000;
			if (edgeOp(c6, c5)) pattern |= 0x40000000;
			c8 = next[2 * xx + 2];
			if (edgeOp(c6, c8)) pattern |= 0x04000000;

			edges2[xx] = pattern;
		}

		pattern = (pattern >> (16 -1)) & 0xA800;
		pattern |= ((edges2[159] >> 5) & 0x01F001F0);

		if (edgeOp(c5, c8)) pattern |= 0x0200;
		Pixel c6 = curr[319];
		if (edgeOp(c6, c8)) pattern |= 0x20001000;
		if (edgeOp(c5, c6)) pattern |= 0x80004000;
		Pixel c9 = next[319];
		if (edgeOp(c5, c9)) pattern |= 0x08000400;

		if (edgeOp(c6, c9)) pattern |= 0x16000000;

		edges2[159] = pattern;
	}
}

template <typename Pixel, typename EdgeOp>
static void calcInitialEdges(
	const Pixel* __restrict srcPrev, const Pixel* __restrict srcCurr,
	unsigned srcWidth, unsigned* __restrict edgeBuf, EdgeOp edgeOp)
{
	unsigned x = 0;
	unsigned c1 = readPixel(srcPrev[x]);
	unsigned c2 = readPixel(srcCurr[x]);
	unsigned pattern = edgeOp(c1, c2) ? ((1 << 6) | (1 << 7)) : 0;
	for (/* */; x < (srcWidth - 1); ++x) {
		pattern >>= 6;
		unsigned n1 = readPixel(srcPrev[x + 1]);
		unsigned n2 = readPixel(srcCurr[x + 1]);
		if (edgeOp(c1, c2)) pattern |= (1 << 5);
		if (edgeOp(c1, n2)) pattern |= (1 << 6);
		if (edgeOp(c2, n1)) pattern |= (1 << 7);
		edgeBuf[x] = pattern;
		c1 = n1; c2 = n2;
	}
	pattern >>= 6;
	if (edgeOp(c1, c2)) pattern |= (1 << 5) | (1 << 6) | (1 << 7);
	edgeBuf[x] = pattern;
}

template <typename Pixel, typename HQScale, typename EdgeOp>
static void doHQScale2(HQScale hqScale, EdgeOp edgeOp, PolyLineScaler<Pixel>& postScale,
	FrameSource& src, unsigned srcStartY, unsigned /*srcEndY*/, unsigned srcWidth,
	ScalerOutput<Pixel>& dst, unsigned dstStartY, unsigned dstEndY, unsigned dstWidth)
{
	int srcY = srcStartY;
	const Pixel* srcPrev = src.getLinePtr<Pixel>(srcY - 1, srcWidth);
	const Pixel* srcCurr = src.getLinePtr<Pixel>(srcY + 0, srcWidth);

	assert(srcWidth <= 1024);
	unsigned edgeBuf[1024];
	calcInitialEdges(srcPrev, srcCurr, srcWidth, edgeBuf, edgeOp);

	bool isCopy = postScale.isCopy();
	for (unsigned dstY = dstStartY; dstY < dstEndY; srcY += 1, dstY += 2) {
		Pixel buf0[2 * 1024], buf1[2 * 1024];
		const Pixel* srcNext = src.getLinePtr<Pixel>(srcY + 1, srcWidth);
		Pixel* dst0 = dst.acquireLine(dstY + 0);
		Pixel* dst1 = dst.acquireLine(dstY + 1);
		if (isCopy) {
			hqScale(srcPrev, srcCurr, srcNext, dst0, dst1,
			      srcWidth, edgeBuf, edgeOp);
		} else {
			hqScale(srcPrev, srcCurr, srcNext, buf0, buf1,
			        srcWidth, edgeBuf, edgeOp);
			postScale(buf0, dst0, dstWidth);
			postScale(buf1, dst1, dstWidth);
		}
		dst.releaseLine(dstY + 0, dst0);
		dst.releaseLine(dstY + 1, dst1);
		srcPrev = srcCurr;
		srcCurr = srcNext;
	}
}

template <typename Pixel, typename HQScale, typename EdgeOp>
static void doHQScale3(HQScale hqScale, EdgeOp edgeOp, PolyLineScaler<Pixel>& postScale,
	FrameSource& src, unsigned srcStartY, unsigned /*srcEndY*/, unsigned srcWidth,
	ScalerOutput<Pixel>& dst, unsigned dstStartY, unsigned dstEndY, unsigned dstWidth)
{
	int srcY = srcStartY;
	const Pixel* srcPrev = src.getLinePtr<Pixel>(srcY - 1, srcWidth);
	const Pixel* srcCurr = src.getLinePtr<Pixel>(srcY + 0, srcWidth);

	assert(srcWidth <= 1024);
	unsigned edgeBuf[1024];
	calcInitialEdges(srcPrev, srcCurr, srcWidth, edgeBuf, edgeOp);

	bool isCopy = postScale.isCopy();
	for (unsigned dstY = dstStartY; dstY < dstEndY; srcY += 1, dstY += 3) {
		Pixel buf0[3 * 1024], buf1[3 * 1024], buf2[3 * 1024];
		const Pixel* srcNext = src.getLinePtr<Pixel>(srcY + 1, srcWidth);
		Pixel* dst0 = dst.acquireLine(dstY + 0);
		Pixel* dst1 = dst.acquireLine(dstY + 1);
		Pixel* dst2 = dst.acquireLine(dstY + 2);
		if (isCopy) {
			hqScale(srcPrev, srcCurr, srcNext, dst0, dst1, dst2,
			        srcWidth, edgeBuf, edgeOp);
		} else {
			hqScale(srcPrev, srcCurr, srcNext, buf0, buf1, buf2,
			        srcWidth, edgeBuf, edgeOp);
			postScale(buf0, dst0, dstWidth);
			postScale(buf1, dst1, dstWidth);
			postScale(buf2, dst2, dstWidth);
		}
		dst.releaseLine(dstY + 0, dst0);
		dst.releaseLine(dstY + 1, dst1);
		dst.releaseLine(dstY + 2, dst2);
		srcPrev = srcCurr;
		srcCurr = srcNext;
	}
}

} // namespace openmsx

#endif
