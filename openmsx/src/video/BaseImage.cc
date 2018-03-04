// $Id: BaseImage.cc 11609 2010-07-22 21:46:22Z m9710797 $

#include "BaseImage.hh"
#include "MSXException.hh"
#include "StringOp.hh"

namespace openmsx {

static const int MAX_SIZE = 2048;

using StringOp::Builder;

void BaseImage::checkSize(int width, int height)
{
	if (width < -MAX_SIZE || width > MAX_SIZE) {
		throw MSXException(
			Builder() << "Image width too large: " << width
			          << " (max " << MAX_SIZE << ')');
	}
	if (height < -MAX_SIZE || height > MAX_SIZE) {
		throw MSXException(
			Builder() << "Image height too large: " << height
			          << " (max " << MAX_SIZE << ')');
	}
}

} // namespace openmsx
