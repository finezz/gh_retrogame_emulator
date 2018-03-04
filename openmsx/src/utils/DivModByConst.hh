// $Id: DivModByConst.hh 12665 2012-06-24 22:05:16Z manuelbi $

#ifndef DIVMODBYCONST
#define DIVMODBYCONST

#include "build-info.hh"

/** Utility class to optimize 64-bit divide/module by a 32-bit constant.
 * For 32-bit by 32-bit gcc already does this optimiztion (on 64-bit
 * CPUs gcc also does it for 64-bit operands). This optimization especially
 * helps on CPU without a HW division instruction (like ARM).
 *
 * Usage:
 *   DivModByConst<123> dm;
 *   unsigned d = dm.div(x);  // equivalent to  d = x / 123;
 *   unsigned m = dm.mod(x);  // equivalent to  d = x % 123;
 */

namespace DivModByConstPrivate {

typedef unsigned long long uint64;

// the cond/endcond stuff is to make sure Doxygen behaves

/** \cond */
template<bool C, class T, class F> struct if_             : F {};
template<        class T, class F> struct if_<true, T, F> : T {};

template<int I> struct int_ { static const int value = I; };
template<unsigned A, unsigned R = 0> struct log2
	: if_<A == 0, int_<R>, log2<A / 2, R + 1> > {};
/** \endcond */

// Utility class to perform 128-bit by 128-bit division at compilation time
template<uint64 RH, uint64 RL, uint64 QH, uint64 QL, uint64 DH, uint64 DL, unsigned BITS>
struct Div128_helper
{
	static const uint64 QL2 = (QL << 1);
	static const uint64 QH2 = (QH << 1) + (QL2 < QL);
	static const uint64 RL2 = (RL << 1) + (QH2 < QH);
	static const uint64 RH2 = (RH << 1) + (RL2 < RL);

	static const bool C = (RH2 != DH) ? (RH2 < DH) : (RL2 < DL);
	static const uint64 RL3 = C ? RL2 : RL2 - DL;
	static const uint64 RH3 = C ? RH2 : RH2 - DH - (RL3 > RL2);
	static const uint64 QL3 = C ? QL2 : QL2 + 1;
	static const uint64 QH3 = C ? QH2 : ((QL3 != 0) ? QH2 : QH2 + 1);

	typedef Div128_helper<RH3, RL3, QH3, QL3, DH, DL, BITS - 1> Div;
	static const uint64 quotientLow   = Div::quotientLow;
	static const uint64 quotientHigh  = Div::quotientHigh;
	static const uint64 remainderLow  = Div::remainderLow;
	static const uint64 remainderHigh = Div::remainderHigh;
};
template<uint64 RH, uint64 RL, uint64 QH, uint64 QL, uint64 DH, uint64 DL>
struct Div128_helper<RH, RL, QH, QL, DH, DL, 0>
{
	static const uint64 quotientLow   = QL;
	static const uint64 quotientHigh  = QH;
	static const uint64 remainderLow  = RL;
	static const uint64 remainderHigh = RH;
};
template<uint64 DividendH, uint64 DividendL, uint64 DividerH, uint64 DividerL>
struct Div128
	: Div128_helper<0, 0, DividendH, DividendL, DividerH, DividerL, 128> {};


// equivalent to the following run-time loop:
//   while (!(M & 1)) {
//       M >>= 1;
//       --S;
//   }
template<uint64 M, unsigned S, bool B = M & 1> struct DBCReduce
{
	typedef DBCReduce<M / 2, S - 1> R2;
	static const uint64   M2 = R2::M2;
	static const unsigned S2 = R2::S2;
};
template<uint64 M, unsigned S> struct DBCReduce<M, S, true>
{
	static const uint64   M2 = M;
	static const unsigned S2 = S;
};

// equivalent to the following run-tim loop:
//   while (((m_low >> 1) < (m_high >> 1)) && (l > 0)) {
//       m_low >>= 1;
//       m_high >>= 1;
//       --l;
//   }
template<uint64 AH, uint64 AL, uint64 BH, uint64 BL>
struct DBCReduce2Shift
{
	static const uint64 AH2 = AH / 2;
	static const uint64 AL2 = AL / 2 + ((AH2 * 2 != AH) ? (1ull << 63) : 0);
	static const uint64 BH2 = BH / 2;
	static const uint64 BL2 = BL / 2 + ((BH2 * 2 != BH) ? (1ull << 63) : 0);
};
template<uint64 AH, uint64 AL, uint64 BH, uint64 BL, unsigned L>
struct DBCReduce2Test
{
	typedef DBCReduce2Shift<AH, AL, BH, BL> S;
	static const bool C = (S::AH2 != S::BH2) ? (S::AH2 < S::BH2)
	                                         : (S::AL2 < S::BL2);
	static const bool value = C && (L > 0);
};
template<uint64 AH, uint64 AL, uint64 BH, uint64 BL, unsigned LL, bool B>
struct DBCReduce2Loop
{
	typedef DBCReduce2Shift<AH, AL, BH, BL> S;
	typedef DBCReduce2Test<S::AH2, S::AL2, S::BH2, S::BL2, LL - 1> T;
	typedef DBCReduce2Loop<S::AH2, S::AL2, S::BH2, S::BL2, LL - 1, T::value> R;
	static const uint64 MLH = R::MLH;
	static const uint64 MLL = R::MLL;
	static const uint64 MHH = R::MHH;
	static const uint64 MHL = R::MHL;
	static const unsigned L = R::L;
};
template<uint64 AH, uint64 AL, uint64 BH, uint64 BL, unsigned LL>
struct DBCReduce2Loop<AH, AL, BH, BL, LL, false>
{
	static const uint64 MLH = AH;
	static const uint64 MLL = AL;
	static const uint64 MHH = BH;
	static const uint64 MHL = BL;
	static const unsigned L = LL;
};
template<uint64 AH, uint64 AL, uint64 BH, uint64 BL, unsigned LL>
struct DBCReduce2
{
	typedef DBCReduce2Test<AH, AL, BH, BL, LL> T;
	typedef DBCReduce2Loop<AH, AL, BH, BL, LL, T::value> R;
	static const uint64 MLH = R::MLH;
	static const uint64 MLL = R::MLL;
	static const uint64 MHH = R::MHH;
	static const uint64 MHL = R::MHL;
	static const unsigned L = R::L;
};


template<unsigned S> struct DBCAlgo1
{
	// division possible by only shifting
	unsigned operator()(uint64 dividend) const
	{
		return dividend >> S;
	}
};

static inline uint64 mla64(uint64 a, uint64 b, uint64 c)
{
	// equivalent to this:
	//    return (__uint128_t(a) * b + c) >> 64;
	uint64 t1 = uint64(unsigned(a)) * unsigned(b);
	uint64 t2 = (a >> 32) * unsigned(b);
	uint64 t3 = unsigned(a) * (b >> 32);
	uint64 t4 = (a >> 32) * (b >> 32);

	uint64 s1 = uint64(unsigned(c)) + unsigned(t1);
	uint64 s2 = (s1 >> 32) + (c >> 32) + (t1 >> 32) + t2;
	uint64 s3 = uint64(unsigned(s2)) + unsigned(t3);
	uint64 s4 = (s3 >> 32) + (s2 >> 32) + (t3 >> 32) + t4;
	return s4;
}

template<uint64 M, unsigned S> struct DBCAlgo2
{
	// division possible by multiplication and shift
	unsigned operator()(uint64 dividend) const
	{
		typedef DBCReduce<M, S> R;
	#if ASM_X86_32 || defined(__arm__)
		const unsigned _ah_ = R::M2 >> 32;
		const unsigned _al_ = unsigned((R::M2 << 32) >> 32); // Suppress VC++ C4310 warning
		const unsigned _bh_ = dividend >> 32;
		const unsigned _bl_ = unsigned(dividend);
	#endif
	#if ASM_X86_32
	#ifdef _MSC_VER
		unsigned _tl_;
		register unsigned result;
		__asm {
			// It's worth noting that simple benchmarks show this to be
			// no faster than straight division on an Intel E8400
			//
			// eax and edx are used with mul
			// ecx = bl
			// esi = ah
			mov		ecx,_bl_
			mov		esi,_ah_
			// ebx is th
			mov     eax,esi
			mul     ecx
			mov     _tl_,eax
			mov     ebx,edx
			mov     eax,_al_
			mul     ecx
			add     _tl_,edx
			adc     ebx,0
			// ecx = bh now
			// edi is cl
			mov		ecx,_bh_
			mov     eax,esi
			mul     ecx
			mov     edi,eax
			// esi is ch now
			mov     esi,edx
			mov     eax,_al_
			mul     ecx
			add     _tl_,eax
			adc     ebx,edx
			adc     esi,0
			add     edi,ebx
			adc     esi,0
			// Sadly, no way to make this an immediate in VC++
			mov		cl,byte ptr [R::S2]
			shrd    edi,esi,cl
			mov		result,edi
		}
	#ifdef DEBUG
		unsigned realResult = unsigned(mla64(dividend, R::M2, 0) >> R::S2);
		assert(realResult == result);
	#endif
		return result;
	#else
		unsigned th, tl, ch, cl;
		asm (
			"movl	%[AH],%%eax\n\t"
			"mull	%[BL]\n\t"
			"movl	%%eax,%[TL]\n\t"
			"movl	%%edx,%[TH]\n\t"
			"movl	%[AL],%%eax\n\t"
			"mull	%[BL]\n\t"
			"addl	%%edx,%[TL]\n\t"
			"adcl	$0,%[TH]\n\t"

			"movl	%[AH],%%eax\n\t"
			"mull	%[BH]\n\t"
			"movl	%%eax,%[CL]\n\t"
			"movl	%%edx,%[CH]\n\t"
			"movl	%[AL],%%eax\n\t"
			"mull	%[BH]\n\t"
			"addl	%%eax,%[TL]\n\t"
			"adcl	%%edx,%[TH]\n\t"
			"adcl	$0,%[CH]\n\t"
			"addl	%[TH],%[CL]\n\t"
			"adcl	$0,%[CH]\n\t"

			: [CH] "=&rm" (ch)
			, [TH] "=&r"  (th)
			, [CL] "=rm"  (cl)
			, [TL] "=&rm" (tl)
			: [AH] "g"    (_ah_)
			, [AL] "g"    (_al_)
			, [BH] "rm"   (_bh_)
			, [BL] "[CL]" (_bl_)
			: "eax","edx"
		);
		asm (
			"shrd   %[SH],%[CH],%[CL]\n\t"

			: [CL] "=rm"  (cl)
			: [CH] "r"    (ch)
			,      "[CL]" (cl)
			, [SH] "i"    (R::S2)
		);
		return cl;
	#endif
	#elif defined(__arm__)
		unsigned res;
		unsigned th,tl;
		asm volatile (
			"umull	%[TH],%[TL],%[AL],%[BL]\n\t"
			"eors	%[TH],%[TH]\n\t"
			"umlal	%[TL],%[TH],%[AH],%[BL]\n\t"

			"umull	%[BL],%[AL],%[BH],%[AL]\n\t"
			"adds	%[TL],%[TL],%[BL]\n\t"
			"adcs	%[TH],%[TH],%[AL]\n\t"
			"mov	%[TL],#0\n\t"
			"adc	%[TL],%[TL],%[TL]\n\t"
			"umlal	%[TH],%[TL],%[AH],%[BH]\n\t"

			"lsr	%[RES],%[TH],%[S]\n\t"
			//"orr	%[RES],%[RES],%[TL],LSL %[S32]\n\t" // not thumb2
			"lsls	%[TL],%[TL],%[S32]\n\t"
			"orrs	%[RES],%[RES],%[TL]\n\t"
			: [RES] "=r"    (res)
			, [TH]  "=&r"   (th)
			, [TL]  "=&r"   (tl)
			: [AH]  "r"     (_ah_)
			, [AL]  "r"     (_al_)
			, [BH]  "r"     (_bh_)
			, [BL]  "[RES]" (_bl_)
			, [S]   "M"   (R::S2)
			, [S32] "M"   (32 - R::S2)
		);
		return res;
	#else
		uint64 h = mla64(dividend, R::M2, 0);
		uint64 result = h >> R::S2;
	#ifdef DEBUG
		// we don't even want this overhead in devel builds
		assert(result == unsigned(result));
	#endif
		return unsigned(result);
	#endif
	}
};

template<unsigned DIVISOR, unsigned N> struct DBCAlgo3
{
	// division possible by multiplication, addition and shift
	static const unsigned S = log2<DIVISOR>::value - 1;
	typedef Div128<1 << S, 0, 0, DIVISOR> D;
	static const uint64 M = D::quotientLow + (D::remainderLow > (DIVISOR / 2));

	unsigned operator()(uint64 dividend) const
	{
		typedef DBCReduce<M, S + N> R;
	#if ASM_X86_32 || defined(__arm__)
		const unsigned ah = R::M2 >> 32;
		const unsigned al = unsigned(R::M2);
		const unsigned bh = dividend >> 32;
		const unsigned bl = dividend;
	#endif
	#if ASM_X86_32
		unsigned th, tl, ch, cl;
		asm (
			"mov	%[AH],%%eax\n\t"
			"mull	%[BL]\n\t"
			"mov	%%eax,%[TL]\n\t"
			"mov	%%edx,%[TH]\n\t"
			"mov	%[AL],%%eax\n\t"
			"mull	%[BL]\n\t"
			"add	%[AL],%%eax\n\t"
			"adc	%[AH],%%edx\n\t"
			"adc	$0,%[TH]\n\t"
			"add	%%edx,%[TL]\n\t"
			"adc	$0,%[TH]\n\t"

			"mov	%[AH],%%eax\n\t"
			"mull	%[BH]\n\t"
			"mov	%%eax,%[CL]\n\t"
			"mov	%%edx,%[CH]\n\t"
			"mov	%[AL],%%eax\n\t"
			"mull	%[BH]\n\t"
			"add	%%eax,%[TL]\n\t"
			"adc	%%edx,%[TH]\n\t"
			"adc	$0,%[CH]\n\t"
			"add	%[TH],%[CL]\n\t"
			"adc	$0,%[CH]\n\t"

			: [CH] "=&rm" (ch)
			, [TH] "=&r"  (th)
			, [CL] "=rm"  (cl)
			, [TL] "=&rm" (tl)
			: [AH] "g"    (ah)
			, [AL] "g"    (al)
			, [BH] "rm"   (bh)
			, [BL] "[CL]" (bl)
			: "eax","edx"
		);
		asm (
			"shrd   %[SH],%[CH],%[CL]\n\t"

			: [CL] "=rm"  (cl)
			: [CH] "r"    (ch)
			,      "[CL]" (cl)
			, [SH] "i"    (R::S2)
		);
		return cl;

	#elif defined(__arm__)
		unsigned res;
		unsigned th,tl;
		asm volatile (
			"umull	%[TH],%[TL],%[AL],%[BL]\n\t"
			"adds	%[TH],%[TH],%[AL]\n\t"
			"adcs	%[TL],%[TL],%[AH]\n\t"
			"mov	%[TH],#0\n\t"
			"adc	%[TH],%[TH],%[TH]\n\t"
			"umlal	%[TL],%[TH],%[AH],%[BL]\n\t"

			"umull	%[BL],%[AL],%[BH],%[AL]\n\t"
			"adds	%[TL],%[TL],%[BL]\n\t"
			"adcs	%[TH],%[TH],%[AL]\n\t"
			"mov	%[TL],#0\n\t"
			"adc	%[TL],%[TL],%[TL]\n\t"
			"umlal	%[TH],%[TL],%[AH],%[BH]\n\t"

			"lsr	%[RES],%[TH],%[S]\n\t"
			//"orr	%[RES],%[RES],%[TL],LSL %[S32]\n\t" // not thumb2
			"lsls	%[TL],%[TL],%[S32]\n\t"
			"orrs	%[RES],%[RES],%[TL]\n\t"
			: [RES] "=r"    (res)
			, [TH]  "=&r"   (th)
			, [TL]  "=&r"   (tl)
			: [AH]  "r"     (ah)
			, [AL]  "r"     (al)
			, [BH]  "r"     (bh)
			, [BL]  "[RES]" (bl)
			, [S]   "M"     (R::S2)
			, [S32] "M"     (32 - R::S2)
		);
		return res;
	#else
		uint64 h = mla64(dividend, R::M2, R::M2);
		return h >> R::S2;
	#endif
	}
};


template<unsigned DIVISOR, unsigned N, typename RM> struct DBCHelper3
	: if_<RM::MHH == 0, DBCAlgo2<RM::MHL, N + RM::L>
	                  , DBCAlgo3<DIVISOR, N> > {};

template<unsigned DIVISOR, unsigned N> struct DBCHelper2
{
	static const unsigned L = log2<DIVISOR>::value;
	static const uint64 J = 0xffffffffffffffffull % DIVISOR;
	typedef Div128<1 << L, 0, 0, 0xffffffffffffffffull - J> K;

	typedef Div128< 1 << L,                    0,              0, DIVISOR> M_LOW;
	typedef Div128<(1 << L) + K::quotientHigh, K::quotientLow, 0, DIVISOR> M_HIGH;
	typedef DBCReduce2<M_LOW ::quotientHigh, M_LOW ::quotientLow,
	                   M_HIGH::quotientHigh, M_HIGH::quotientLow, L> R;

	unsigned operator()(uint64 dividend) const
	{
		DBCHelper3<DIVISOR, N, R> dbc;
		return dbc(dividend);
	}
};

template<unsigned DIVISOR, unsigned SHIFT> struct DBCHelper1
	: if_<DIVISOR == 1, DBCAlgo1<SHIFT>,
	                    if_<DIVISOR & 1, DBCHelper2<DIVISOR, SHIFT>
	                                   , DBCHelper1<DIVISOR / 2, SHIFT + 1> > > {};

} // namespace DivModByConstPrivate


template<unsigned DIVISOR> struct DivModByConst
{
	unsigned div(unsigned long long dividend) const
	{
	#ifdef __x86_64
		// on 64-bit CPUs gcc already does this
		// optimization (and better)
		return unsigned(dividend / DIVISOR);
	#else
		DivModByConstPrivate::DBCHelper1<DIVISOR, 0> dbc;
		return dbc(dividend);
	#endif
	}

	unsigned mod(unsigned long long dividend) const
	{
		unsigned long long result;
	#ifdef __x86_64
		result = dividend % DIVISOR;
	#else
		result = dividend - DIVISOR * div(dividend);
	#endif
	#ifdef DEBUG
		// we don't even want this overhead in devel builds
		assert(result == unsigned(result));
	#endif
		return unsigned(result);
	}
};

#endif // DIVMODBYCONST
