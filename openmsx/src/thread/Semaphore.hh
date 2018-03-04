// $Id: Semaphore.hh 5509 2006-07-10 20:59:33Z m9710797 $

#ifndef SEMAPHORE_HH
#define SEMAPHORE_HH

#include "noncopyable.hh"
#include <SDL.h>
#include <cassert>

namespace openmsx {

class Semaphore : private noncopyable
{
public:
	explicit Semaphore(unsigned value);
	~Semaphore();
	void up();
	void down();

private:
	SDL_sem* semaphore;
};

class ScopedLock : private noncopyable
{
public:
	ScopedLock()
		: lock(NULL)
	{
	}

	explicit ScopedLock(Semaphore& lock_)
		: lock(NULL)
	{
		take(lock_);
	}

	~ScopedLock()
	{
		release();
	}

	void take(Semaphore& lock_)
	{
		assert(!lock);
		lock = &lock_;
		lock->down();
	}

	void release()
	{
		if (lock) {
			lock->up();
			lock = NULL;
		}
	}

private:
	Semaphore* lock;
};

} // namespace openmsx

#endif
