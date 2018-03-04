// $Id: Subject.hh 11797 2010-12-08 18:10:17Z m9710797 $

#ifndef SUBJECT_HH
#define SUBJECT_HH

#include "Observer.hh"
#include "ScopedAssign.hh"
#include <algorithm>
#include <vector>
#include <cassert>

namespace openmsx {

/**
 * Generic Gang-of-Four Subject class of the Observer pattern, templatized
 * edition.
 */
template <typename T> class Subject
{
public:
	void attach(Observer<T>& observer);
	void detach(Observer<T>& observer);

protected:
	Subject();
	~Subject();
	void notify() const;

private:
	typedef std::vector<Observer<T>*> Observers;
	Observers observers;
#ifndef NDEBUG
	mutable bool notifyInProgress;
#endif
};

template <typename T> Subject<T>::Subject()
#ifndef NDEBUG
	: notifyInProgress(false)
#endif
{
}

template <typename T> Subject<T>::~Subject()
{
	assert(!notifyInProgress);
	Observers copy(observers);
	for (typename Observers::const_iterator it = copy.begin();
	     it != copy.end(); ++it) {
		(*it)->subjectDeleted(*static_cast<const T*>(this));
	}
	assert(observers.empty());
}

template <typename T> void Subject<T>::attach(Observer<T>& observer)
{
	assert(!notifyInProgress);
	observers.push_back(&observer);
}

template <typename T> void Subject<T>::detach(Observer<T>& observer)
{
	assert(!notifyInProgress);
	typename Observers::iterator it =
		find(observers.begin(), observers.end(), &observer);
	assert(it != observers.end());
	observers.erase(it);
}

template <typename T> void Subject<T>::notify() const
{
#ifndef NDEBUG
	assert(!notifyInProgress);
	ScopedAssign<bool> sa(notifyInProgress, true);
#endif

	for (typename Observers::const_iterator it = observers.begin();
			it != observers.end(); ++it) {
		(*it)->update(*static_cast<const T*>(this));
	}
}

} // namespace openmsx

#endif
