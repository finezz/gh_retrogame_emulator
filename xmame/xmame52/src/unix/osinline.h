#ifndef __OSINLINE__
#define __OSINLINE__

/* for uclock() */
#include "sysdep/misc.h"

#define osd_cycles() uclock()

#if defined svgalib || defined x11 || defined ggi || defined openstep || defined SDL
extern unsigned char *dirty_lines;
extern unsigned char **dirty_blocks;

#define osd_mark_vector_dirty(x,y) \
{ \
   dirty_lines[(y)>>3] = 1; \
   dirty_blocks[(y)>>3][(x)>>3] = 1; \
}

#else
#define osd_mark_vector_dirty(x,y)
#endif

#ifdef X86_ASM
#define vec_mult _vec_mult
INLINE int _vec_mult(int x, int y)
{
	int result;
	asm (
			"movl  %1    , %0    ; "
			"imull %2            ; "    /* do the multiply */
			"movl  %%edx , %%eax ; "
			:  "=&a" (result)           /* the result has to go in eax */
			:  "mr" (x),                /* x and y can be regs or mem */
			   "mr" (y)
			:  "%edx", "%cc"            /* clobbers edx and flags */
		);
	return result;
}
#endif

#ifdef USE_MIPS_ASSEMBLER
#define vec_mult _vec_mult
INLINE int _vec_mult(int x, int y)
{
        int result;

	__asm__ __volatile__
	(
 	 "mult %1,%2 \n"
	 "mfhi %0 \n"
	: "=r"(result)
	: "r"(x), "r"(y)
	);

	return result;
}


#define vec_div _vec_div
INLINE int _vec_div(int x, int y)
{
	int result=0x00010000;
	if (y>>12)
	{
	    __asm__ __volatile__
	    (
	     "sll %1,%1,4 \n"
	     "srl %2,%2,12 \n"
	     "div %1,%2 \n"
	     "mflo %0 \n"
	    : "=r"(result)
	    : "r"(x), "r"(y)
	    );
	    if( result > 0x00010000 )
		return( 0x00010000 );
	    if( result < -0x00010000 )
		return( -0x00010000 );
	}

	return result;
}

#endif /* USE_MIPS_ASSEMBLER */


#endif /* __OSINLINE__ */
