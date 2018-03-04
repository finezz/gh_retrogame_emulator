// $Id: aligned.hh 10516 2009-09-05 17:37:57Z mthuurne $

#ifndef ALIGNED_HH
#define ALIGNED_HH

#ifdef _MSC_VER
#define ALIGNED(EXPRESSION, ALIGNMENT) __declspec (align(ALIGNMENT)) EXPRESSION
#else // GCC style
#define ALIGNED(EXPRESSION, ALIGNMENT) EXPRESSION __attribute__((__aligned__((ALIGNMENT))));
#endif

#endif // ALIGNED_HH
