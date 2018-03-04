// $Id: CassetteImage.hh 12587 2012-06-06 20:31:32Z m9710797 $

#ifndef CASSETTEIMAGE_HH
#define CASSETTEIMAGE_HH

#include "EmuTime.hh"
#include "sha1.hh"
#include <string>

namespace openmsx {

class CassetteImage
{
public:
	enum FileType { ASCII, BINARY, BASIC, UNKNOWN };

	virtual ~CassetteImage();
	virtual short getSampleAt(EmuTime::param time) = 0;
	virtual EmuTime getEndTime() const = 0;
	virtual unsigned getFrequency() const = 0;
	virtual void fillBuffer(unsigned pos, int** bufs, unsigned num) const = 0;

	FileType getFirstFileType() const;
	std::string getFirstFileTypeAsString() const;

	/** Get sha1sum for this image.
	 * This is based on the content of the file, not the logical meaning of
	 * the file. IOW: it's possible for different files (with different
	 * sha1sum) to represent the same logical cassette data (e.g. wav with
	 * different bits per sample). This method will give a different
	 * sha1sum to such files.
	 */
	const Sha1Sum& getSha1Sum() const;

protected:
	CassetteImage();
	void setFirstFileType(FileType type);
	void setSha1Sum(const Sha1Sum& sha1sum);

private:
	FileType firstFileType;
	Sha1Sum sha1sum;
};

} // namespace openmsx

#endif
