// $Id: noncopyable.hh 6672 2007-06-27 14:31:15Z m9710797 $

#ifndef NONCOPYABLE_HH
#define NONCOPYABLE_HH

/**
 * Based on boost::noncopyable, see boost documentation:
 *   http://www.boost.org/libs/utility
 *
 * Summary:
 *  Class noncopyable is a base class. Derive your own class from noncopyable
 *  when you want to prohibit copy construction and copy assignment.
 */
class noncopyable
{
protected:
	noncopyable() {}
	~noncopyable() {}
private:
	noncopyable(const noncopyable&);
	const noncopyable& operator=(const noncopyable&);
};

#endif
