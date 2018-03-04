// $Id: Simple2xScaler.cc 12814 2012-08-13 20:22:45Z m9710797 $

#include "Simple2xScaler.hh"
#include "SuperImposedVideoFrame.hh"
#include "LineScalers.hh"
#include "RawFrame.hh"
#include "ScalerOutput.hh"
#include "RenderSettings.hh"
#include "HostCPU.hh"
#include "openmsx.hh"
#include "vla.hh"
#include "build-info.hh"
#include <cassert>

namespace openmsx {

// class Simple2xScaler

template <class Pixel>
Simple2xScaler<Pixel>::Simple2xScaler(
		const PixelOperations<Pixel>& pixelOps_,
		RenderSettings& renderSettings)
	: Scaler2<Pixel>(pixelOps_)
	, settings(renderSettings)
	, pixelOps(pixelOps_)
	, mult1(pixelOps)
	, mult2(pixelOps)
	, mult3(pixelOps)
	, scanline(pixelOps)
{
}

template <class Pixel>
void Simple2xScaler<Pixel>::scaleBlank1to2(
		FrameSource& src, unsigned srcStartY, unsigned srcEndY,
		ScalerOutput<Pixel>& dst, unsigned dstStartY, unsigned dstEndY)
{
	int scanlineFactor = settings.getScanlineFactor();

	unsigned dstHeight = dst.getHeight();
	unsigned stopDstY = (dstEndY == dstHeight)
	                  ? dstEndY : dstEndY - 2;
	unsigned srcY = srcStartY, dstY = dstStartY;
	for (/* */; dstY < stopDstY; srcY += 1, dstY += 2) {
		Pixel color0 = src.getLinePtr<Pixel>(srcY)[0];
		dst.fillLine(dstY + 0, color0);
		Pixel color1 = scanline.darken(color0, scanlineFactor);
		dst.fillLine(dstY + 1, color1);
	}
	if (dstY != dstHeight) {
		unsigned nextLineWidth = src.getLineWidth(srcY + 1);
		assert(src.getLineWidth(srcY) == 1);
		assert(nextLineWidth != 1);
		this->dispatchScale(src, srcY, srcEndY, nextLineWidth,
		                    dst, dstY, dstEndY);
	}
}

// Assembly functions
#ifdef _MSC_VER
extern "C"
{
	void __cdecl Simple2xScaler_blur1on2_4_MMX(
		const void* pIn, void* pOut, unsigned long srcWidth,
		unsigned c1, unsigned c2);
	void __cdecl Simple2xScaler_blur1on1_4_MMX(
		const void* pIn, void* pOut, unsigned long srcWidth,
		unsigned c1, unsigned c2);
}
#endif

template <class Pixel>
void Simple2xScaler<Pixel>::blur1on2(
	const Pixel* __restrict pIn, Pixel* __restrict pOut,
	unsigned alpha, unsigned long srcWidth)
{
	/* This routine is functionally equivalent to the following:
	 *
	 * void blur1on2(const Pixel* pIn, Pixel* pOut, unsigned alpha)
	 * {
	 *         unsigned c1 = alpha;
	 *         unsigned c2 = 256 - c1;
	 *
	 *         Pixel prev, curr, next;
	 *         prev = curr = pIn[0];
	 *
	 *         unsigned x;
	 *         for (x = 0; x < (srcWidth - 1); ++x) {
	 *                 pOut[2 * x + 0] = (c1 * prev + c2 * curr) >> 8;
	 *                 Pixel next = pIn[x + 1];
	 *                 pOut[2 * x + 1] = (c1 * next + c2 * curr) >> 8;
	 *                 prev = curr;
	 *                 curr = next;
	 *         }
	 *
	 *         pOut[2 * x + 0] = (c1 * prev + c2 * curr) >> 8;
	 *         next = curr;
	 *         pOut[2 * x + 1] = (c1 * next + c2 * curr) >> 8;
	 * }
	 *
	 * The loop is 2x unrolled and all common subexpressions and redundant
	 * assignments have been eliminated. 1 loop iteration processes 4
	 * (output) pixels.
	 */

	if (alpha == 0) {
		Scale_1on2<Pixel, false> scale; // no streaming stores
		scale(pIn, pOut, 2 * srcWidth);
		return;
	}

	assert(alpha <= 256);
	unsigned c1 = alpha / 4;
	unsigned c2 = 256 - c1;

	#if ASM_X86
	if ((sizeof(Pixel) == 4) && HostCPU::hasMMX()) { // Note: not hasMMXEXT()
		// MMX routine, 32bpp
		assert(((srcWidth * 4) % 8) == 0);
	#ifdef _MSC_VER
		Simple2xScaler_blur1on2_4_MMX(pIn, pOut, srcWidth, c1, c2);
	#else
		unsigned long dummy;
		asm volatile (
			"movd	%[C1], %%mm5;"
			"punpcklwd %%mm5, %%mm5;"
			"punpckldq %%mm5, %%mm5;"	// mm5 = c1
			"movd	%[C2], %%mm6;"
			"punpcklwd %%mm6, %%mm6;"
			"punpckldq %%mm6, %%mm6;"	// mm6 = c2
			"pxor	%%mm7, %%mm7;"

			"movd	(%[IN],%[CNT]), %%mm0;"
			"punpcklbw %%mm7, %%mm0;"	// p0 = pIn[0]
			"movq	%%mm0, %%mm2;"
			"pmullw	%%mm5, %%mm2;"		// f0 = multiply(p0, c1)
			"movq	%%mm2, %%mm3;"		// f1 = f0

			".p2align 4,,15;"
		"1:"
			"pmullw	%%mm6, %%mm0;"
			"movq	%%mm0, %%mm4;"		// tmp = multiply(p0, c2)
			"paddw	%%mm3, %%mm0;"
			"psrlw	$8, %%mm0;"		// f1 + tmp

			"movd	4(%[IN],%[CNT]), %%mm1;"
			"punpcklbw %%mm7, %%mm1;"	// p1 = pIn[x + 1]
			"movq	%%mm1, %%mm3;"
			"pmullw	%%mm5, %%mm3;"		// f1 = multiply(p1, c1)
			"paddw	%%mm3, %%mm4;"
			"psrlw	$8, %%mm4;"		// f1 + tmp
			"packuswb %%mm4, %%mm0;"
			"movq	%%mm0, (%[OUT],%[CNT],2);"	// pOut[2*x+0] = ..  pOut[2*x+1] = ..

			"pmullw	%%mm6, %%mm1;"
			"movq	%%mm1, %%mm4;"		// tmp = multiply(p1, c2)
			"paddw	%%mm2, %%mm1;"
			"psrlw	$8, %%mm1;"		// f0 + tmp

			"movd	8(%[IN],%[CNT]), %%mm0;"
			"punpcklbw %%mm7, %%mm0;"	// p0 = pIn[x + 2]
			"movq	%%mm0, %%mm2;"
			"pmullw %%mm5, %%mm2;"		// f0 = multiply(p0, c1)
			"paddw	%%mm2, %%mm4;"
			"psrlw	$8, %%mm4;"		// f0 + tmp
			"packuswb %%mm4, %%mm1;"
			"movq	%%mm1, 8(%[OUT],%[CNT],2);"	// pOut[2*x+2] = ..  pOut[2*x+3] = ..

			"add	$8, %[CNT];"
			"jnz	1b;"

			"pmullw	%%mm6, %%mm0;"
			"movq	%%mm0, %%mm4;"		// tmp = multiply(p0, c2)
			"paddw	%%mm3, %%mm0;"
			"psrlw	$8, %%mm0;"		// f1 + tmp

			"movd	4(%[IN]), %%mm1;"
			"punpcklbw %%mm7, %%mm1;"	// p1 = pIn[x + 1]
			"movq	%%mm1, %%mm3;"
			"pmullw	%%mm5, %%mm3;"		// f1 = multiply(p1, c1)
			"paddw	%%mm3, %%mm4;"
			"psrlw	$8, %%mm4;"		// f1 + tmp
			"packuswb %%mm4, %%mm0;"
			"movq	%%mm0, (%[OUT]);"	// pOut[2*x+0] = ..  pOut[2*x+1] = ..

			"movq	%%mm1, %%mm4;"
			"pmullw	%%mm6, %%mm1;"		// tmp = multiply(p1, c2)
			"paddw	%%mm2, %%mm1;"
			"psrlw	$8, %%mm1;"		// f0 + tmp

			"packuswb %%mm4, %%mm1;"
			"movq	%%mm1, 8(%[OUT]);"	// pOut[2*x+0] = ..  pOut[2*x+1] = ..

			"emms;"

			: [CNT] "=r"    (dummy)
			: [IN]  "r"     (pIn  +     (srcWidth - 2))
			, [OUT] "r"     (pOut + 2 * (srcWidth - 2))
			, [C1]  "r"     (c1)
			, [C2]  "r"     (c2)
			,       "[CNT]" (-4 * (srcWidth - 2))
			: "memory"
			#ifdef __MMX__
			, "mm0", "mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm7"
			#endif
		);
	#endif
		return;
	}
	#endif

	// non-MMX routine, both 16bpp and 32bpp
	mult1.setFactor32(c1);
	mult2.setFactor32(c2);

	Pixel p0 = pIn[0];
	Pixel p1;
	unsigned f0 = mult1.mul32(p0);
	unsigned f1 = f0;
	unsigned tmp;

	unsigned x;
	for (x = 0; x < (srcWidth - 2); x += 2) {
		tmp = mult2.mul32(p0);
		pOut[2 * x + 0] = mult1.conv32(f1 + tmp);

		p1 = pIn[x + 1];
		f1 = mult1.mul32(p1);
		pOut[2 * x + 1] = mult1.conv32(f1 + tmp);

		tmp = mult2.mul32(p1);
		pOut[2 * x + 2] = mult1.conv32(f0 + tmp);

		p0 = pIn[x + 2];
		f0 = mult1.mul32(p0);
		pOut[2 * x + 3] = mult1.conv32(f0 + tmp);
	}

	tmp = mult2.mul32(p0);
	pOut[2 * x + 0] = mult1.conv32(f1 + tmp);

	p1 = pIn[x + 1];
	f1 = mult1.mul32(p1);
	pOut[2 * x + 1] = mult1.conv32(f1 + tmp);

	tmp = mult2.mul32(p1);
	pOut[2 * x + 2] = mult1.conv32(f0 + tmp);

	pOut[2 * x + 3] = p1;
}

template <class Pixel>
void Simple2xScaler<Pixel>::blur1on1(
	const Pixel* __restrict pIn, Pixel* __restrict pOut,
	unsigned alpha, unsigned long srcWidth)
{
	/* This routine is functionally equivalent to the following:
	 *
	 * void blur1on1(const Pixel* pIn, Pixel* pOut, unsigned alpha)
	 * {
	 *         unsigned c1 = alpha / 2;
	 *         unsigned c2 = 256 - alpha;
	 *
	 *         Pixel prev, curr, next;
	 *         prev = curr = pIn[0];
	 *
	 *         unsigned x;
	 *         for (x = 0; x < (srcWidth - 1); ++x) {
	 *                 next = pIn[x + 1];
	 *                 pOut[x] = (c1 * prev + c2 * curr + c1 * next) >> 8;
	 *                 prev = curr;
	 *                 curr = next;
	 *         }
	 *
	 *         next = curr;
	 *         pOut[x] = c1 * prev + c2 * curr + c1 * next;
	 * }
	 *
	 * The loop is 2x unrolled and all common subexpressions and redundant
	 * assignments have been eliminated. 1 loop iteration processes 2
	 * pixels.
	 */

	if (alpha == 0) {
		Scale_1on1<Pixel, false> copy; // no streaming stores
		copy(pIn, pOut, srcWidth);
		return;
	}

	unsigned c1 = alpha / 4;
	unsigned c2 = 256 - alpha / 2;

	#if ASM_X86
	if ((sizeof(Pixel) == 4) && HostCPU::hasMMX()) { // Note: not hasMMXEXT()
		// MMX routine, 32bpp
		assert(((srcWidth * 4) % 8) == 0);
	#ifdef _MSC_VER
		Simple2xScaler_blur1on1_4_MMX(pIn, pOut, srcWidth, c1, c2);
	#else
		unsigned long dummy;
		asm volatile (
			"movd	%[C1], %%mm5;"
			"punpcklwd %%mm5, %%mm5;"
			"punpckldq %%mm5, %%mm5;"	// mm5 = c1
			"movd	%[C2], %%mm6;"
			"punpcklwd %%mm6, %%mm6;"
			"punpckldq %%mm6, %%mm6;"	// mm6 = c2
			"pxor	%%mm7, %%mm7;"

			"movd	(%[IN],%[CNT]), %%mm0;"
			"punpcklbw %%mm7, %%mm0;"	// p0 = pIn[0]
			"movq	%%mm0, %%mm2;"
			"pmullw	%%mm5, %%mm2;"		// f0 = multiply(p0, c1)
			"movq	%%mm2, %%mm3;"		// f1 = f0

			".p2align 4,,15;"
		"1:"
			"movd	4(%[IN],%[CNT]), %%mm1;"
			"pxor	%%mm7, %%mm7;"
			"punpcklbw %%mm7, %%mm1;"	// p1 = pIn[x + 1]
			"movq	%%mm0, %%mm4;"
			"pmullw	%%mm6, %%mm4;"		// t = multiply(p0, c2)
			"movq	%%mm1, %%mm0;"
			"pmullw	%%mm5, %%mm0;"		// t0 = multiply(p1, c1)
			"paddw	%%mm2, %%mm4;"
			"paddw  %%mm0, %%mm4;"
			"psrlw	$8, %%mm4;"		// f0 + t + t0
			"movq	%%mm0, %%mm2;"		// f0 = t0

			"movd	8(%[IN],%[CNT]), %%mm0;"
			"punpcklbw %%mm7, %%mm0;"
			"movq	%%mm1, %%mm7;"
			"pmullw	%%mm6, %%mm7;"		// t = multiply(p1, c2)
			"movq	%%mm0, %%mm1;"
			"pmullw %%mm5, %%mm1;"		// t1 = multiply(p0, c1)
			"paddw	%%mm3, %%mm7;"
			"paddw	%%mm1, %%mm7;"
			"psrlw	$8, %%mm7;"		// f1 + t + t1
			"movq	%%mm1, %%mm3;"		// f1 = t1
			"packuswb %%mm7, %%mm4;"
			"movq	%%mm4, (%[OUT],%[CNT]);"	// pOut[x] = ..  pOut[x+1] = ..

			"add	$8, %[CNT];"
			"jnz	1b;"

			"movd	4(%[IN]), %%mm1;"
			"pxor	%%mm7, %%mm7;"
			"punpcklbw %%mm7, %%mm1;"	// p1 = pIn[x + 1]
			"movq	%%mm0, %%mm4;"
			"pmullw	%%mm6, %%mm4;"		// t = multiply(p0, c2)
			"movq	%%mm1, %%mm0;"
			"pmullw	%%mm5, %%mm0;"		// t0 = multiply(p1, c1)
			"paddw	%%mm2, %%mm4;"
			"paddw  %%mm0, %%mm4;"
			"psrlw	$8, %%mm4;"		// f0 + t + t0

			"pmullw	%%mm6, %%mm1;"		// t = multiply(p1, c2)
			"paddw	%%mm3, %%mm1;"
			"paddw	%%mm0, %%mm1;"
			"psrlw	$8, %%mm1;"		// f1 + t + t1
			"packuswb %%mm1, %%mm4;"
			"movq	%%mm4, (%[OUT]);"	// pOut[x] = ..  pOut[x+1] = ..

			"emms;"

			: [CNT] "=r"    (dummy)
			: [IN]  "r"     (pIn  + srcWidth - 2)
			, [OUT] "r"     (pOut + srcWidth - 2)
			, [C1]  "r"     (c1)
			, [C2]  "r"     (c2)
			,       "[CNT]" (-4 * (srcWidth - 2))
			: "memory"
			#ifdef __MMX__
			, "mm0", "mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm7"
			#endif
		);
	#endif
		return;
	}
	#endif

	mult1.setFactor32(c1);
	mult3.setFactor32(c2);

	Pixel p0 = pIn[0];
	Pixel p1;
	unsigned f0 = mult1.mul32(p0);
	unsigned f1 = f0;

	unsigned x;
	for (x = 0; x < (srcWidth - 2); x += 2) {
		p1 = pIn[x + 1];
		unsigned t0 = mult1.mul32(p1);
		pOut[x] = mult1.conv32(f0 + mult3.mul32(p0) + t0);
		f0 = t0;

		p0 = pIn[x + 2];
		unsigned t1 = mult1.mul32(p0);
		pOut[x + 1] = mult1.conv32(f1 + mult3.mul32(p1) + t1);
		f1 = t1;
	}

	p1 = pIn[x + 1];
	unsigned t0 = mult1.mul32(p1);
	pOut[x] = mult1.conv32(f0 + mult3.mul32(p0) + t0);

	pOut[x + 1] = mult1.conv32(f1 + mult3.mul32(p1) + t0);
}

template <class Pixel>
void Simple2xScaler<Pixel>::drawScanline(
		const Pixel* in1, const Pixel* in2, Pixel* out, int factor,
		unsigned dstWidth)
{
	if (factor != 255) {
		scanline.draw(in1, in2, out, factor, dstWidth);
	} else {
		Scale_1on1<Pixel> scale;
		scale(in1, out, dstWidth);
	}
}

template <class Pixel>
void Simple2xScaler<Pixel>::scale1x1to2x2(FrameSource& src,
	unsigned srcStartY, unsigned /*srcEndY*/, unsigned srcWidth,
	ScalerOutput<Pixel>& dst, unsigned dstStartY, unsigned dstEndY)
{
	int blur = settings.getBlurFactor();
	int scanlineFactor = settings.getScanlineFactor();

	unsigned dstY = dstStartY;
	const Pixel* srcLine = src.getLinePtr<Pixel>(srcStartY++, srcWidth);
	Pixel* dstLine0 = dst.acquireLine(dstY + 0);
	blur1on2(srcLine, dstLine0, blur, srcWidth);

	for (/**/; dstY < dstEndY - 2; dstY += 2) {
		srcLine = src.getLinePtr<Pixel>(srcStartY++, srcWidth);
		Pixel* dstLine2 = dst.acquireLine(dstY + 2);
		blur1on2(srcLine, dstLine2, blur, srcWidth);

		Pixel* dstLine1 = dst.acquireLine(dstY + 1);
		drawScanline(dstLine0, dstLine2, dstLine1, scanlineFactor,
		             2 * srcWidth);

		dst.releaseLine(dstY + 0, dstLine0);
		dst.releaseLine(dstY + 1, dstLine1);
		dstLine0 = dstLine2;
	}

	srcLine = src.getLinePtr<Pixel>(srcStartY++, srcWidth);
	VLA_SSE_ALIGNED(Pixel, buf, 2 * srcWidth);
	blur1on2(srcLine, buf, blur, srcWidth);

	Pixel* dstLine1 = dst.acquireLine(dstY + 1);
	drawScanline(dstLine0, buf, dstLine1, scanlineFactor, 2 * srcWidth);
	dst.releaseLine(dstY + 0, dstLine0);
	dst.releaseLine(dstY + 1, dstLine1);
}

template <class Pixel>
void Simple2xScaler<Pixel>::scale1x1to1x2(FrameSource& src,
	unsigned srcStartY, unsigned /*srcEndY*/, unsigned srcWidth,
	ScalerOutput<Pixel>& dst, unsigned dstStartY, unsigned dstEndY)
{
	int blur = settings.getBlurFactor();
	int scanlineFactor = settings.getScanlineFactor();

	unsigned dstY = dstStartY;
	const Pixel* srcLine = src.getLinePtr<Pixel>(srcStartY++, srcWidth);
	Pixel* dstLine0 = dst.acquireLine(dstY);
	blur1on1(srcLine, dstLine0, blur, srcWidth);

	for (/**/; dstY < dstEndY - 2; dstY += 2) {
		srcLine = src.getLinePtr<Pixel>(srcStartY++, srcWidth);
		Pixel* dstLine2 = dst.acquireLine(dstY + 2);
		blur1on1(srcLine, dstLine2, blur, srcWidth);

		Pixel* dstLine1 = dst.acquireLine(dstY + 1);
		drawScanline(dstLine0, dstLine2, dstLine1, scanlineFactor,
		             srcWidth);

		dst.releaseLine(dstY + 0, dstLine0);
		dst.releaseLine(dstY + 1, dstLine1);
		dstLine0 = dstLine2;
	}

	srcLine = src.getLinePtr<Pixel>(srcStartY++, srcWidth);
	VLA_SSE_ALIGNED(Pixel, buf, srcWidth);
	blur1on1(srcLine, buf, blur, srcWidth);

	Pixel* dstLine1 = dst.acquireLine(dstY + 1);
	drawScanline(dstLine0, buf, dstLine1, scanlineFactor, srcWidth);
	dst.releaseLine(dstY + 0, dstLine0);
	dst.releaseLine(dstY + 1, dstLine1);
}

template <class Pixel>
void Simple2xScaler<Pixel>::scaleImage(
	FrameSource& src, const RawFrame* superImpose,
	unsigned srcStartY, unsigned srcEndY, unsigned srcWidth,
	ScalerOutput<Pixel>& dst, unsigned dstStartY, unsigned dstEndY)
{
	if (superImpose) {
		// Note: this implementation is different from the openGL
		// version. Here we first alpha-blend and then scale, so the
		// video layer will also get blurred (and possibly down-scaled
		// to MSX resolution). The openGL version will only blur the
		// MSX frame, then blend with the video frame and then apply
		// scanlines. I think the openGL version is visually slightly
		// better, but much more work to implement in software (in
		// openGL shaders it's very easy). Maybe we can improve this
		// later (if required at all).
		SuperImposedVideoFrame<Pixel> sf(src, *superImpose, pixelOps);
		srcWidth = sf.getLineWidth(srcStartY);
		this->dispatchScale(sf,  srcStartY, srcEndY, srcWidth,
		                    dst, dstStartY, dstEndY);
		src.freeLineBuffers();
		superImpose->freeLineBuffers();
	} else {
		this->dispatchScale(src, srcStartY, srcEndY, srcWidth,
		                    dst, dstStartY, dstEndY);
	}
}

// Force template instantiation.
#if HAVE_16BPP
template class Simple2xScaler<word>;
#endif
#if HAVE_32BPP
template class Simple2xScaler<unsigned>;
#endif

} // namespace openmsx
