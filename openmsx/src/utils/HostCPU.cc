// $Id: HostCPU.cc 12814 2012-08-13 20:22:45Z m9710797 $

#include "HostCPU.hh"
#include "openmsx.hh"
#include <cassert>
#ifdef _MSC_VER
#include <intrin.h>
#endif

namespace HostCPU {

// Initially assume no CPU feature support
bool mmxFlag  = false;
bool sseFlag  = false;
bool sse2Flag = false;

/*static*/ // to avoid 'unsused static function' compiler warning on x86_64
void setFeatures(unsigned features)
{
	const unsigned CPUID_FEATURE_MMX  = 0x00800000;
	const unsigned CPUID_FEATURE_SSE  = 0x02000000;
	const unsigned CPUID_FEATURE_SSE2 = 0x04000000;

	mmxFlag  = (features & CPUID_FEATURE_MMX)  != 0;
	sseFlag  = (features & CPUID_FEATURE_SSE)  != 0;
	sse2Flag = (features & CPUID_FEATURE_SSE2) != 0;
}

void init()
{
	#ifdef __x86_64
	// X86_64 machines always have mmx, sse, sse2
	mmxFlag  = true;
	sseFlag  = true;
	sse2Flag = true;
	#elif ASM_X86_32
	#ifdef _MSC_VER
	unsigned hasCPUID;
	__asm {
		// Load EFLAGS into EAX
		pushfd
		pop		eax
		// Save current value.
		mov		ecx,eax
		// Toggle bit 21.
		xor		eax,200000h
		// Load EAX into EFLAGS.
		push	eax
		popfd
		// Load EFLAGS into EAX.
		pushfd
		pop		eax
		// Did bit 21 change?
		xor		eax,ecx
		and		eax,200000h
		mov		hasCPUID,eax
	}
	if (hasCPUID) {
		int cpuInfo[4];
		__cpuid(cpuInfo, 0);
		if (cpuInfo[0] >= 1) {
			__cpuid(cpuInfo, 1);
			setFeatures(cpuInfo[3]);
		}
	}
	#else
		// Note: On Mac OS X, EBX is in use by the OS,
		//       so we have to restore it.
		// Is CPUID instruction supported?
		unsigned hasCPUID;
		asm (
			// Load EFLAGS into EAX.
			"pushfl;"
			"popl	%%eax;"
			// Save current value.
			"movl	%%eax,%%ecx;"
			// Toggle bit 21.
			"xorl	$0x200000, %%eax;"
			// Load EAX into EFLAGS.
			"pushl	%%eax;"
			"popfl;"
			// Load EFLAGS into EAX.
			"pushfl;"
			"popl	%%eax;"
			// Did bit 21 change?
			"xor	%%ecx, %%eax;"
			"andl	$0x200000, %%eax;"
			: "=a" (hasCPUID) // 0
			: // no input
			: "ecx"
			);
		if (hasCPUID) {
			// Which CPUID calls are supported?
			unsigned highest;
			asm (
				"pushl	%%ebx;"
				"cpuid;"
				"popl	%%ebx;"
				: "=a" (highest) // 0
				: "0" (0) // 1: function
				: "ecx", "edx"
			);
			if (highest >= 1) {
				// Get features flags.
				unsigned features;
				asm (
					"pushl	%%ebx;"
					"cpuid;"
					"popl	%%ebx;"
					: "=d" (features) // 0
					: "a" (1) // 1: function
					: "ecx"
				);
				setFeatures(features);
			}
		}
	#endif
	#endif

	PRT_DEBUG("MMX:  " << mmxFlag);
	PRT_DEBUG("SSE:  " << sseFlag);
	PRT_DEBUG("SSE2: " << sse2Flag);

	if (hasSSE2()) { assert(hasMMX() && hasSSE()); }
	if (hasSSE())  { assert(hasMMX()); }
}

} // namespace openmsx
