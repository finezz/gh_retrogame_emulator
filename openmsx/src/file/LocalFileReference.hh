// $Id: LocalFileReference.hh 11787 2010-12-03 14:47:56Z m9710797 $

#ifndef LOCALFILEREFERENCE_HH
#define LOCALFILEREFERENCE_HH

#include "Filename.hh"
#include "noncopyable.hh"
#include <string>

namespace openmsx {

class File;

/** Helper class to use files is APIs other than openmsx::File.
 * The openMSX File class has support for (g)zipped files (or maybe in the
 * future files over http, ftp, ...). Sometimes you need to pass a filename
 * to an API that doesn't support this (for example SDL_LoadWav()). This
 * class allows to create a temporary local uncompressed version of such
 * files. Use it like this:
 *
 *   LocalFileReference file(filename);  // can be any filename supported
 *                                       // by openmsx::File
 *   my_function(file.getFilename());    // my_function() can now work on
 *                                       // a regular local file
 *
 * Note: In the past this functionality was available in the openmsx::File
 *       class. The current implementation of that class always keep an open
 *       file reference to the corresponding file. This gave problems on
 *       (some versions of) windows if the external function tries to open
 *       the file in read-write mode (for example IMG_Load() does this). The
 *       implementation of this class does not keep a reference to the file.
 */
class LocalFileReference : private noncopyable
{
public:
	explicit LocalFileReference(const Filename& filename);
	explicit LocalFileReference(const std::string& url);
	explicit LocalFileReference(File& file);
	~LocalFileReference();

	/** Returns path to a local uncompressed version of this file.
	  * This path only remains valid as long as this object is in scope.
	  */
	const std::string getFilename() const;

private:
	void init(const std::string& url);
	void init(File& url);

	std::string tmpFile;
	std::string tmpDir;
};

} // namespace openmsx

#endif
