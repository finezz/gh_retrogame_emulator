// $Id: Timer.cc 11659 2010-09-01 17:24:42Z m9710797 $

#include "Timer.hh"
#include "systemfuncs.hh"
#if HAVE_CLOCK_GETTIME
#include <ctime>
#endif
#if HAVE_USLEEP
#include <unistdp.hh>
#endif
#if defined _WIN32
#include <windows.h>
#endif
#include <SDL.h>
#include <cassert>

namespace openmsx {

namespace Timer {

static inline unsigned long long getSDLTicks()
{
	return static_cast<unsigned long long>(SDL_GetTicks()) * 1000;
}

unsigned long long getTime()
{
	static unsigned long long lastTime = 0;
	unsigned long long now;
/* QueryPerformanceCounter() has problems on modern CPUs,
 *  - on dual core CPUs time can ge backwards (a bit) when your process
 *    get scheduled on the other core
 *  - the resolution of the timer can vary on CPUs that can change its
 *    clock frequency (for power managment)
##if defined _WIN32
	static LONGLONG hfFrequency = 0;

	LARGE_INTEGER li;
	if (!hfFrequency) {
		if (QueryPerformanceFrequency(&li)) {
			hfFrequency = li.QuadPart;
		} else {
			return getSDLTicks();
		}
	}
	QueryPerformanceCounter(&li);

	// Assumes that the timer never wraps. The mask is just to
	// ensure that the multiplication doesn't wrap.
	now = (li.QuadPart & ((long long)-1 >> 20)) * 1000000 / hfFrequency;
*/
#if HAVE_CLOCK_GETTIME && defined(_POSIX_MONOTONIC_CLOCK)
	// Note: in the past we used the more portable gettimeofday() function,
	//       but the result of that function is not always monotonic.
	timespec ts;
	int result = clock_gettime(CLOCK_MONOTONIC, &ts);
	assert(result == 0); (void)result;
	now = static_cast<unsigned long long>(ts.tv_sec) * 1000000 +
	      static_cast<unsigned long long>(ts.tv_nsec) / 1000;
#else
	now = getSDLTicks();
#endif
	if (now < lastTime) {
		// This shouldn't happen, time should never go backwards.
		// Though there appears to be a bug in some Linux kernels
		// so that occasionally clock_gettime(CLOCK_MONOTONIC) _does_
		// go back in time slightly. When that happens we return the
		// last time again.
		return lastTime;
	}
	lastTime = now;
	return now;
}

/*#if defined _WIN32
static void CALLBACK timerCallback(unsigned int,
                                   unsigned int,
                                   unsigned long eventHandle,
                                   unsigned long,
                                   unsigned long)
{
    SetEvent((HANDLE)eventHandle);
}
#endif*/

void sleep(unsigned long long us)
{
/*#if defined _WIN32
	us /= 1000;
	if (us > 0) {
		static HANDLE timerEvent = NULL;
		if (timerEvent == NULL) {
			timerEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		}
		UINT id = timeSetEvent(us, 1, timerCallback, (DWORD)timerEvent,
		                       TIME_ONESHOT);
		WaitForSingleObject(timerEvent, INFINITE);
		timeKillEvent(id);
	}
*/
#if HAVE_USLEEP
	usleep(us);
#else
	SDL_Delay(unsigned(us / 1000));
#endif
}

} // namespace Timer

} // namespace openmsx
