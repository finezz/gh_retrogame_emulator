// $Id: Scanline.cc 12814 2012-08-13 20:22:45Z m9710797 $

#include "Scanline.hh"
#include "PixelOperations.hh"
#include "HostCPU.hh"
#include "unreachable.hh"
#include "build-info.hh"
#include <cassert>
#include <cstring>

namespace openmsx {

// class Multiply<word>

Multiply<word>::Multiply(const PixelOperations<word>& pixelOps_)
	: pixelOps(pixelOps_)
{
	factor = 0;
	memset(tab, 0, sizeof(tab));
}

void Multiply<word>::setFactor(unsigned f)
{
	if (f == factor) {
		return;
	}
	factor = f;

	for (unsigned p = 0; p < 0x10000; ++p) {
		tab[p] = ((((p & pixelOps.getRmask()) * f) >> 8) & pixelOps.getRmask()) |
		         ((((p & pixelOps.getGmask()) * f) >> 8) & pixelOps.getGmask()) |
		         ((((p & pixelOps.getBmask()) * f) >> 8) & pixelOps.getBmask());
	}
}

inline word Multiply<word>::multiply(word p, unsigned f) const
{
	unsigned r = (((p & pixelOps.getRmask()) * f) >> 8) & pixelOps.getRmask();
	unsigned g = (((p & pixelOps.getGmask()) * f) >> 8) & pixelOps.getGmask();
	unsigned b = (((p & pixelOps.getBmask()) * f) >> 8) & pixelOps.getBmask();
	return r | g | b;
}

inline word Multiply<word>::multiply(word p) const
{
	return tab[p];
}

inline const word* Multiply<word>::getTable() const
{
	return tab;
}


// class Multiply<unsigned>

Multiply<unsigned>::Multiply(const PixelOperations<unsigned>& /*pixelOps*/)
{
}

void Multiply<unsigned>::setFactor(unsigned f)
{
	factor = f;
}

inline unsigned Multiply<unsigned>::multiply(unsigned p, unsigned f) const
{
	return PixelOperations<unsigned>::multiply(p, f);
}

inline unsigned Multiply<unsigned>::multiply(unsigned p) const
{
	return multiply(p, factor);
}

const unsigned* Multiply<unsigned>::getTable() const
{
	UNREACHABLE; return NULL;
}


// class Scanline

// Assembly functions
#ifdef _MSC_VER
extern "C"
{
	void __cdecl Scanline_draw_4_SSE2(const void* src1, const void* src2,
		void* dst, unsigned factor, unsigned long width);
}
#endif

template <class Pixel>
Scanline<Pixel>::Scanline(const PixelOperations<Pixel>& pixelOps_)
	: darkener(pixelOps_)
	, pixelOps(pixelOps_)
{
}

template <class Pixel>
void Scanline<Pixel>::draw(
	const Pixel* __restrict src1, const Pixel* __restrict src2,
	Pixel* __restrict dst, unsigned factor, unsigned long width)
{
#if ASM_X86
#ifdef _MSC_VER
	if ((sizeof(Pixel) == 4) && HostCPU::hasSSE2()) {
		// SSE2 routine, 32bpp
		assert(((4 * width) % 64) == 0);
		Scanline_draw_4_SSE2(src1, src2, dst, factor, width);
		return;
	}
#else
	if ((sizeof(Pixel) == 4) && HostCPU::hasSSE2()) {
		// SSE2 routine, 32bpp
		assert(((4 * width) % 64) == 0);
		unsigned long dummy;
		asm volatile (
			"movd	%[F], %%xmm6;"
			"pshuflw $0, %%xmm6, %%xmm6;"
			"pxor	%%xmm7, %%xmm7;"
			"pshufd  $0, %%xmm6, %%xmm6;"
			".p2align 4,,15;"
		"1:"
			"movdqa	(%[IN1],%[CNT]), %%xmm0;"
			"pavgb	(%[IN2],%[CNT]), %%xmm0;"
			"movdqa	%%xmm0, %%xmm4;"
			"punpcklbw %%xmm7, %%xmm0;"
			"punpckhbw %%xmm7, %%xmm4;"
			"pmulhuw %%xmm6, %%xmm0;"
			"pmulhuw %%xmm6, %%xmm4;"
			"packuswb %%xmm4, %%xmm0;"

			"movdqa	16(%[IN1],%[CNT]), %%xmm1;"
			"pavgb	16(%[IN2],%[CNT]), %%xmm1;"
			"movdqa	%%xmm1, %%xmm5;"
			"punpcklbw %%xmm7, %%xmm1;"
			"punpckhbw %%xmm7, %%xmm5;"
			"pmulhuw %%xmm6, %%xmm1;"
			"pmulhuw %%xmm6, %%xmm5;"
			"packuswb %%xmm5, %%xmm1;"

			"movdqa	32(%[IN1],%[CNT]), %%xmm2;"
			"pavgb	32(%[IN2],%[CNT]), %%xmm2;"
			"movdqa	%%xmm2, %%xmm4;"
			"punpcklbw %%xmm7, %%xmm2;"
			"punpckhbw %%xmm7, %%xmm4;"
			"pmulhuw %%xmm6, %%xmm2;"
			"pmulhuw %%xmm6, %%xmm4;"
			"packuswb %%xmm4, %%xmm2;"

			"movdqa	48(%[IN1],%[CNT]), %%xmm3;"
			"pavgb	48(%[IN2],%[CNT]), %%xmm3;"
			"movdqa	%%xmm3, %%xmm5;"
			"punpcklbw %%xmm7, %%xmm3;"
			"punpckhbw %%xmm7, %%xmm5;"
			"pmulhuw %%xmm6, %%xmm3;"
			"pmulhuw %%xmm6, %%xmm5;"
			"packuswb %%xmm5, %%xmm3;"

			"movntps %%xmm0,   (%[OUT],%[CNT]);"
			"movntps %%xmm1, 16(%[OUT],%[CNT]);"
			"movntps %%xmm2, 32(%[OUT],%[CNT]);"
			"movntps %%xmm3, 48(%[OUT],%[CNT]);"

			"add	$64, %[CNT];"
			"jnz	1b;"

			: [CNT] "=r"    (dummy)
			: [IN1] "r"     (src1 + width)
			, [IN2] "r"     (src2 + width)
			, [OUT] "r"     (dst  + width)
			, [F]   "r"     (factor << 8)
			,       "[CNT]" (-4 * width)
			: "memory"
			#ifdef __SSE__
			, "xmm0", "xmm1", "xmm2", "xmm3"
			, "xmm4", "xmm5", "xmm6", "xmm7"
			#endif
		);
		return;

	} else if ((sizeof(Pixel) == 4) && HostCPU::hasSSE()) {
		// extended-MMX routine, 32bpp
		assert(((4 * width) % 32) == 0);
		unsigned long dummy;
		asm volatile (
			"movd	%[F], %%mm6;"
			"pxor	%%mm7, %%mm7;"
			"pshufw $0, %%mm6, %%mm6;"
			".p2align 4,,15;"
		"1:"
			"movq	(%[IN1],%[CNT]), %%mm0;"
			"pavgb	(%[IN2],%[CNT]), %%mm0;"
			"movq	%%mm0, %%mm4;"
			"punpcklbw %%mm7, %%mm0;"
			"punpckhbw %%mm7, %%mm4;"
			"pmulhuw %%mm6, %%mm0;"
			"pmulhuw %%mm6, %%mm4;"
			"packuswb %%mm4, %%mm0;"

			"movq	8(%[IN1],%[CNT]), %%mm1;"
			"pavgb	8(%[IN2],%[CNT]), %%mm1;"
			"movq	%%mm1, %%mm5;"
			"punpcklbw %%mm7, %%mm1;"
			"punpckhbw %%mm7, %%mm5;"
			"pmulhuw %%mm6, %%mm1;"
			"pmulhuw %%mm6, %%mm5;"
			"packuswb %%mm5, %%mm1;"

			"movq	16(%[IN1],%[CNT]), %%mm2;"
			"pavgb	16(%[IN2],%[CNT]), %%mm2;"
			"movq	%%mm2, %%mm4;"
			"punpcklbw %%mm7, %%mm2;"
			"punpckhbw %%mm7, %%mm4;"
			"pmulhuw %%mm6, %%mm2;"
			"pmulhuw %%mm6, %%mm4;"
			"packuswb %%mm4, %%mm2;"

			"movq	24(%[IN1],%[CNT]), %%mm3;"
			"pavgb	24(%[IN2],%[CNT]), %%mm3;"
			"movq	%%mm3, %%mm5;"
			"punpcklbw %%mm7, %%mm3;"
			"punpckhbw %%mm7, %%mm5;"
			"pmulhuw %%mm6, %%mm3;"
			"pmulhuw %%mm6, %%mm5;"
			"packuswb %%mm5, %%mm3;"

			"movntq %%mm0,   (%[OUT],%[CNT]);"
			"movntq %%mm1,  8(%[OUT],%[CNT]);"
			"movntq %%mm2, 16(%[OUT],%[CNT]);"
			"movntq %%mm3, 24(%[OUT],%[CNT]);"

			"add	$32, %[CNT];"
			"jnz	1b;"

			"emms;"

			: [CNT] "=r"    (dummy)
			: [IN1] "r"     (src1 + width)
			, [IN2] "r"     (src2 + width)
			, [OUT] "r"     (dst  + width)
			, [F]   "r"     (factor << 8)
			,       "[CNT]" (-4 * width)
			: "memory"
			#ifdef __MMX__
			, "mm0", "mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm7"
			#endif
		);
		return;

	} else if ((sizeof(Pixel) == 4) && HostCPU::hasMMX()) {
		// MMX routine, 32bpp
		assert(((4 * width) % 8) == 0);
		unsigned long dummy;
		asm volatile (
			"movd	%[F], %%mm6;"
			"pxor	%%mm7, %%mm7;"
			"punpcklwd %%mm6, %%mm6;"
			"punpckldq %%mm6, %%mm6;"
			".p2align 4,,15;"
		"1:"
			// load
			"movq	(%[IN1],%[CNT]), %%mm0;"
			"movq	%%mm0, %%mm1;"
			"movq	(%[IN2],%[CNT]), %%mm2;"
			"movq	%%mm2, %%mm3;"
			// unpack
			"punpcklbw %%mm7, %%mm0;"
			"punpckhbw %%mm7, %%mm1;"
			"punpcklbw %%mm7, %%mm2;"
			"punpckhbw %%mm7, %%mm3;"
			// average
			"paddw	%%mm2, %%mm0;"
			"paddw	%%mm3, %%mm1;"
			// darken
			"pmulhw	%%mm6, %%mm0;"
			"pmulhw	%%mm6, %%mm1;"
			// pack
			"packuswb %%mm1, %%mm0;"
			// store
			"movq %%mm0, (%[OUT],%[CNT]);"

			"add	$8, %[CNT];"
			"jnz	1b;"

			"emms;"

			: [CNT] "=r"    (dummy)
			: [IN1] "r"     (src1 + width)
			, [IN2] "r"     (src2 + width)
			, [OUT] "r"     (dst + width)
			, [F]   "r"     (factor << 7)
			,       "[CNT]" (-4 * width)
			: "memory"
			#ifdef __MMX__
			, "mm0", "mm1", "mm2", "mm3", "mm6", "mm7"
			#endif
		);
		return;
	}

	#if !defined(__APPLE__) && !ASM_X86_64
	// On Mac OS X, we are one register short, because EBX is not available.
	// We disable this piece of assembly and fall back to the C++ code.
	// It's unlikely modern Macs will be running in 16bpp anyway.
	if ((sizeof(Pixel) == 2) && HostCPU::hasSSE()) {
		// extended-MMX routine, 16bpp
		assert(((2 * width) % 16) == 0);

		darkener.setFactor(factor);
		const Pixel* table = darkener.getTable();
		Pixel mask = pixelOps.getBlendMask();

		unsigned long dummy;
		asm volatile (
			"movd	%[MASK], %%mm7;"
			"pshufw	$0, %%mm7, %%mm7;"

			".p2align 4,,15;"
		"1:"	"movq	 (%[IN1],%[CNT]), %%mm0;"
			"movq	8(%[IN1],%[CNT]), %%mm1;"
			"movq	 (%[IN2],%[CNT]), %%mm2;"
			"movq	8(%[IN2],%[CNT]), %%mm3;"

			"movq	%%mm7, %%mm4;"
			"movq	%%mm7, %%mm5;"
			"pand	%%mm7, %%mm0;"
			"pand	%%mm7, %%mm1;"
			"pandn  %%mm2, %%mm4;"
			"pandn  %%mm3, %%mm5;"
			"pand	%%mm7, %%mm2;"
			"pand	%%mm7, %%mm3;"
			"pavgw	%%mm2, %%mm0;"
			"pavgw	%%mm3, %%mm1;"
			"paddw	%%mm4, %%mm0;"
			"paddw	%%mm5, %%mm1;"

			"pextrw	$0, %%mm0, %%eax;"
			"movw	(%[TAB],%%eax,2), %%ax;"
			"pinsrw	$0, %%eax, %%mm0;"
			"pextrw	$0, %%mm1, %%eax;"
			"movw	(%[TAB],%%eax,2), %%ax;"
			"pinsrw	$0, %%eax, %%mm1;"

			"pextrw	$1, %%mm0, %%eax;"
			"movw	(%[TAB],%%eax,2), %%ax;"
			"pinsrw	$1, %%eax, %%mm0;"
			"pextrw	$1, %%mm1, %%eax;"
			"movw	(%[TAB],%%eax,2), %%ax;"
			"pinsrw	$1, %%eax, %%mm1;"

			"pextrw	$2, %%mm0, %%eax;"
			"movw	(%[TAB],%%eax,2), %%ax;"
			"pinsrw	$2, %%eax, %%mm0;"
			"pextrw	$2, %%mm1, %%eax;"
			"movw	(%[TAB],%%eax,2), %%ax;"
			"pinsrw	$2, %%eax, %%mm1;"

			"pextrw	$3, %%mm0, %%eax;"
			"movw	(%[TAB],%%eax,2), %%ax;"
			"pinsrw	$3, %%eax, %%mm0;"
			"pextrw	$3, %%mm1, %%eax;"
			"movw	(%[TAB],%%eax,2), %%ax;"
			"pinsrw	$3, %%eax, %%mm1;"

			"movntq	%%mm0,  (%[OUT],%[CNT]);"
			"movntq	%%mm1, 8(%[OUT],%[CNT]);"

			"add	$16, %[CNT];"
			"jnz	1b;"
			"emms;"
			: [CNT]  "=r"    (dummy)
			: [IN1]  "r"     (src1 + width)
			, [IN2]  "r"     (src2 + width)
			, [TAB]  "r"     (table)
			, [OUT]  "r"     (dst + width)
			, [MASK] "m"     (mask)
			,        "[CNT]" (-2 * width)
			: "eax"
			#ifdef __MMX__
			, "mm0", "mm1", "mm2", "mm3", "mm4", "mm5", "mm7"
			#endif
		);
		return;
	}
	#endif // !__APPLE__
	// MMX routine 16bpp is missing, but it's difficult to write because
	// of the missing "pextrw" and "pinsrw" instructions

	#endif
	#endif

	// non-MMX routine, both 16bpp and 32bpp
	darkener.setFactor(factor);
	for (unsigned x = 0; x < width; ++x) {
		dst[x] = darkener.multiply(
			pixelOps.template blend<1, 1>(src1[x], src2[x]));
	}
}

template <class Pixel>
Pixel Scanline<Pixel>::darken(Pixel p, unsigned factor)
{
	return darkener.multiply(p, factor);
}

template <class Pixel>
Pixel Scanline<Pixel>::darken(Pixel p1, Pixel p2, unsigned factor)
{
	return darkener.multiply(pixelOps.template blend<1, 1>(p1, p2), factor);
}

// Force template instantiation.
#if HAVE_16BPP
template class Scanline<word>;
#endif
#if HAVE_32BPP
template class Scanline<unsigned>;
#endif

} // namespace openmsx
