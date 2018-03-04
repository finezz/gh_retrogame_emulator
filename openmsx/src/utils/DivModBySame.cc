// $Id: DivModBySame.cc 12668 2012-06-25 15:24:51Z mthuurne $

#include "DivModBySame.hh"
#include "uint128.hh"
#include <cassert>


namespace openmsx {

static unsigned log2(uint64 i)
{
	unsigned t = 0;
	i >>= 1;
	while (i) {
		i >>= 1;
		++t;
	}
	return t;
}

void DivModBySame::setDivisor(unsigned divisor_)
{
	//assert(divisor_ < 0x8000000000000000ull); // when divisor is uint64
	divisor = divisor_;

	// reduce divisor until it becomes odd
	unsigned n = 0;
	uint64 t = divisor;
	while (!(t & 1)) {
		t >>= 1;
		++n;
	}
	if (t == 1) {
		m = 0xffffffffffffffffull;
		a = m;
		s = 0;
	} else {
		// Generate m, s for algorithm 0. Based on: Granlund, T.; Montgomery,
		// P.L.: "Division by Invariant Integers using Multiplication".
		// SIGPLAN Notices, Vol. 29, June 1994, page 61.
		unsigned l = log2(t) + 1;
		uint64 j = 0xffffffffffffffffull % t;
		uint128 k = (uint128(1) << (64 + l)) / (0xffffffffffffffffull - j);
		uint128 m_low  =  (uint128(1) << (64 + l))      / t;
		uint128 m_high = ((uint128(1) << (64 + l)) + k) / t;
		while (((m_low >> 1) < (m_high >> 1)) && (l > 0)) {
			m_low >>= 1;
			m_high >>= 1;
			--l;
		}
		if ((m_high >> 64) == 0) {
			m = toUint64(m_high);
			s = l;
			a = 0;
		} else {
			// Generate m, s for algorithm 1. Based on: Magenheimer, D.J.; et al:
			// "Integer Multiplication and Division on the HP Precision Architecture".
			// IEEE Transactions on Computers, Vol 37, No. 8, August 1988, page 980.
			s = log2(t);
			uint128 m_low =     (uint128(1) << (64 + s)) / t;
			uint64 r = toUint64((uint128(1) << (64 + s)) % t);
			m = toUint64(m_low + ((r <= (t >> 1)) ? 0 : 1));
			a = m;
		}
		// reduce multiplier to smallest possible
		while (!(m & 1)) {
			m >>= 1;
			a >>= 1;
			s--;
		}
	}
	// adjust multiplier for reduction of even divisors
	s += n;
}

} // namespace openmsx
