// $Id: SDLImage.hh 11623 2010-08-09 17:42:13Z m9710797 $

#ifndef SDLIMAGE_HH
#define SDLIMAGE_HH

#include "BaseImage.hh"
#include "SDLSurfacePtr.hh"
#include <string>

namespace openmsx {

class SDLImage : public BaseImage
{
public:
	explicit SDLImage(const std::string& filename);
	explicit SDLImage(SDLSurfacePtr image);
	SDLImage(const std::string& filename, double scaleFactor);
	SDLImage(const std::string& filename, int width, int height);
	SDLImage(int width, int height, unsigned rgba);
	SDLImage(int width, int height, const unsigned* rgba,
	         unsigned borderSize, unsigned borderRGBA);

	virtual void draw(OutputSurface& output, int x, int y,
	                  byte alpha = 255);
	virtual int getWidth() const;
	virtual int getHeight() const;

private:
	void initSolid(int width, int height, unsigned rgba,
	               unsigned borderSize, unsigned borderRGBA);
	void initGradient(int width, int height, const unsigned* rgba,
	               unsigned borderSize, unsigned borderRGBA);
	void allocateWorkImage();

	SDLSurfacePtr image;
	SDLSurfacePtr workImage;
	int a; // whole surface alpha value, -1 if per-pixel alpha
	const bool flipX, flipY;
};

} // namespace openmsx

#endif
