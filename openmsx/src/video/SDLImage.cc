// $Id: SDLImage.cc 12591 2012-06-06 20:32:44Z m9710797 $

#include "SDLImage.hh"
#include "PNG.hh"
#include "OutputSurface.hh"
#include "PixelOperations.hh"
#include "MSXException.hh"
#include "vla.hh"
#include "unreachable.hh"
#include "build-info.hh"
#if PLATFORM_GP2X
#include "GP2XMMUHack.hh"
#endif
#include <cassert>
#include <cstdlib>
#include <SDL.h>

using std::string;

namespace openmsx {

static bool hasConstantAlpha(const SDL_Surface& surface, byte& alpha)
{
	unsigned amask = surface.format->Amask;
	if (amask == 0) {
		// If there's no alpha layer, the surface has a constant
		// opaque alpha value.
		alpha = SDL_ALPHA_OPAQUE;
		return true;
	}

	// There is an alpha layer, surface must be 32bpp.
	assert(surface.format->BitsPerPixel == 32);
	assert(surface.format->Aloss == 0);

	// Compare alpha from each pixel. Are they all the same?
	const unsigned* data = reinterpret_cast<const unsigned*>(surface.pixels);
	unsigned alpha0 = data[0] & amask;
	for (int y = 0; y < surface.h; ++y) {
		const unsigned* p = data + y * (surface.pitch / sizeof(unsigned));
		for (int x = 0; x < surface.w; ++x) {
			if ((p[x] & amask) != alpha0) return false;
		}
	}

	// The alpha value of each pixel is constant, get that value.
	alpha = alpha0 >> surface.format->Ashift;
	return true;
}

static SDLSurfacePtr convertToDisplayFormat(SDLSurfacePtr input)
{
	SDL_PixelFormat& inFormat  = *input->format;
	SDL_PixelFormat& outFormat = *SDL_GetVideoSurface()->format;
	assert((inFormat.BitsPerPixel == 24) || (inFormat.BitsPerPixel == 32));

	byte alpha;
	if (hasConstantAlpha(*input, alpha)) {
		Uint32 flags = (alpha == SDL_ALPHA_OPAQUE) ? 0 : SDL_SRCALPHA;
		SDL_SetAlpha(input.get(), flags, alpha);
		if (inFormat.BitsPerPixel == outFormat.BitsPerPixel) {
			assert(inFormat.Rmask == outFormat.Rmask);
			assert(inFormat.Gmask == outFormat.Gmask);
			assert(inFormat.Bmask == outFormat.Bmask);
			return input;
		}
		// 32bpp should never need this conversion
		assert(outFormat.BitsPerPixel != 32);
		return SDLSurfacePtr(SDL_DisplayFormat(input.get()));
	} else {
		assert(inFormat.Amask != 0);
		assert(inFormat.BitsPerPixel == 32);
		// We need an alpha channel, so leave the image in 32bpp format.
		return input;
	}
}

static void zoomSurface(const SDL_Surface* src, SDL_Surface* dst,
                        bool flipX, bool flipY)
{
	assert(src->format->BitsPerPixel == 32);
	assert(dst->format->BitsPerPixel == 32);

	PixelOperations<unsigned> pixelOps(*dst->format);

	// For interpolation: assume source dimension is one pixel
	// smaller to avoid overflow on right and bottom edge.
	int sx = int(65536.0 * double(src->w - 1) / double(dst->w));
	int sy = int(65536.0 * double(src->h - 1) / double(dst->h));

	// Interpolating Zoom, Scan destination
	const unsigned* sp = static_cast<const unsigned*>(src->pixels);
	      unsigned* dp = static_cast<      unsigned*>(dst->pixels);
	int srcPitch = src->pitch / sizeof(unsigned);
	int dstPitch = dst->pitch / sizeof(unsigned);
	if (flipY) dp += (dst->h - 1) * dstPitch;
	for (int y = 0, csy = 0; y < dst->h; ++y, csy += sy) {
		sp += (csy >> 16) * srcPitch;
		const unsigned* c00 = sp;
		const unsigned* c10 = sp + srcPitch;
		csy &= 0xffff;
		if (!flipX) {
			// not horizontally mirrored
			for (int x = 0, csx = 0; x < dst->w; ++x, csx += sx) {
				int sstep = csx >> 16;
				c00 += sstep;
				c10 += sstep;
				csx &= 0xffff;
				// Interpolate RGBA
				unsigned t1 = pixelOps.lerp(c00[0], c00[1], (csx >> 8));
				unsigned t2 = pixelOps.lerp(c10[0], c10[1], (csx >> 8));
				dp[x] = pixelOps.lerp(t1 , t2 , (csy >> 8));
			}
		} else {
			// horizontally mirrored
			for (int x = dst->w - 1, csx = 0; x >= 0; --x, csx += sx) {
				int sstep = csx >> 16;
				c00 += sstep;
				c10 += sstep;
				csx &= 0xffff;
				// Interpolate RGBA
				unsigned t1 = pixelOps.lerp(c00[0], c00[1], (csx >> 8));
				unsigned t2 = pixelOps.lerp(c10[0], c10[1], (csx >> 8));
				dp[x] = pixelOps.lerp(t1 , t2 , (csy >> 8));
			}
		}
		dp += flipY ? -dstPitch : dstPitch;
	}
}

static void getRGBAmasks32(Uint32& rmask, Uint32& gmask, Uint32& bmask, Uint32& amask)
{
	SDL_PixelFormat& format = *SDL_GetVideoSurface()->format;
	if ((format.BitsPerPixel == 32) && (format.Rloss == 0) &&
	    (format.Gloss == 0) && (format.Bloss == 0)) {
		rmask = format.Rmask;
		gmask = format.Gmask;
		bmask = format.Bmask;
		// on a display surface Amask is often 0, so instead
		// we use the bits that are not yet used for RGB
		//amask = format.Amask;
		amask = ~(rmask | gmask | bmask);
		assert((amask == 0x000000ff) || (amask == 0x0000ff00) ||
		       (amask == 0x00ff0000) || (amask == 0xff000000));
	} else {
		// ARGB8888 (this seems to be the 'default' format in SDL)
		amask = 0xff000000;
		rmask = 0x00ff0000;
		gmask = 0x0000ff00;
		bmask = 0x000000ff;
	}
}

static SDLSurfacePtr create32BppSurface(int width, int height, bool alpha)
{
	Uint32 rmask, gmask, bmask, amask;
	getRGBAmasks32(rmask, gmask, bmask, amask);
	if (!alpha) amask = 0;

	return SDLSurfacePtr(
		abs(width), abs(height), 32, rmask, gmask, bmask, amask);
}

static SDLSurfacePtr scaleImage32(SDLSurfacePtr input, int width, int height)
{
	// create a 32 bpp surface that will hold the scaled version
	assert(input->format->BitsPerPixel == 32);
	bool alpha = input->format->Amask != 0;
	SDLSurfacePtr result = create32BppSurface(width, height, alpha);
	zoomSurface(input.get(), result.get(), width < 0, height < 0);
	return result;
}

static SDLSurfacePtr loadImage(const string& filename)
{
	// If the output surface is 32bpp, then always load the PNG as
	// 32bpp (even if it has no alpha channel).
	bool want32bpp = SDL_GetVideoSurface()->format->BitsPerPixel == 32;
	return convertToDisplayFormat(PNG::load(filename, want32bpp));
}

static SDLSurfacePtr loadImage(const string& filename, double scaleFactor)
{
	if (scaleFactor == 1.0) {
		return loadImage(filename);
	}
	bool want32bpp = true; // scaleImage32 needs 32bpp
	SDLSurfacePtr picture(PNG::load(filename, want32bpp));
	int width  = int(picture->w * scaleFactor);
	int height = int(picture->h * scaleFactor);
	BaseImage::checkSize(width, height);
	if ((width == 0) || (height == 0)) {
		return SDLSurfacePtr();
	}
	return convertToDisplayFormat(scaleImage32(picture, width, height));
}

static SDLSurfacePtr loadImage(
	const string& filename, int width, int height)
{
	BaseImage::checkSize(width, height);
	if ((width == 0) || (height == 0)) {
		return SDLSurfacePtr();
	}
	bool want32bpp = true; // scaleImage32 needs 32bpp
	SDLSurfacePtr picture(PNG::load(filename, want32bpp));
	return convertToDisplayFormat(scaleImage32(picture, width, height));
}

// Helper functions to draw a gradient
//  Extract R,G,B,A components to 8.16 bit fixed point.
//  Note the order R,G,B,A is arbitrary, the actual pixel value may have the
//  components in a different order.
static void unpackRGBA(unsigned rgba,
                       unsigned& r, unsigned&g, unsigned&b, unsigned& a)
{
	r = (((rgba >> 24) & 0xFF) << 16) + 0x8000;
	g = (((rgba >> 16) & 0xFF) << 16) + 0x8000;
	b = (((rgba >>  8) & 0xFF) << 16) + 0x8000;
	a = (((rgba >>  0) & 0xFF) << 16) + 0x8000;
}
// Setup outer loop (vertical) interpolation parameters.
//  For each component there is a pair of (initial,delta) values. These values
//  are 8.16 bit fixed point, delta is signed.
static void setupInterp1(unsigned rgba0, unsigned rgba1, unsigned length,
                         unsigned& r0, unsigned& g0, unsigned& b0, unsigned& a0,
                         int& dr, int& dg, int& db, int& da)
{
	unpackRGBA(rgba0, r0, g0, b0, a0);
	if (length == 1) {
		dr = dg = db = da = 0;
	} else {
		unsigned r1, g1, b1, a1;
		unpackRGBA(rgba1, r1, g1, b1, a1);
		dr = int(r1 - r0) / int(length - 1);
		dg = int(g1 - g0) / int(length - 1);
		db = int(b1 - b0) / int(length - 1);
		da = int(a1 - a0) / int(length - 1);
	}
}
// Setup inner loop (horizontal) interpolation parameters.
// - Like above we also output a pair of (initial,delta) values for each
//   component. But we pack two components in one 32-bit value. This leaves only
//   16 bits per component, so now the values are 8.8 bit fixed point.
// - To avoid carry/borrow from the lower to the upper pack, we make the lower
//   component always a positive number and output a boolean to indicate whether
//   we should add or subtract the delta from the initial value.
// - The 8.8 fixed point calculations in the inner loop are less accurate than
//   the 8.16 calculations in the outer loop. This could result in not 100%
//   accurate gradients. Though only on very wide images and the error is
//   so small that it will hardly be visible (if at all).
// - Packing 2 components in one value is not beneficial in the outer loop
//   because in this routine we need the individual components of the values
//   that are calculated by setupInterp1(). (It would also make the code even
//   more complex).
static void setupInterp2(unsigned r0, unsigned g0, unsigned b0, unsigned a0,
                         unsigned r1, unsigned g1, unsigned b1, unsigned a1,
                         unsigned length,
                         unsigned&  rb, unsigned&  ga,
                         unsigned& drb, unsigned& dga,
                         bool&   subRB, bool&   subGA)
{
	// Pack the initial values for the components R,B and G,A into
	// a vector-type: two 8.16 scalars -> one [8.8 ; 8.8] vector
	rb = ((r0 << 8) & 0xffff0000) |
	     ((b0 >> 8) & 0x0000ffff);
	ga = ((g0 << 8) & 0xffff0000) |
	     ((a0 >> 8) & 0x0000ffff);
	subRB = subGA = false;
	if (length == 1) {
		drb = dga = 0;
	} else {
		// calculate delta values
		int dr = int(r1 - r0) / int(length - 1);
		int dg = int(g1 - g0) / int(length - 1);
		int db = int(b1 - b0) / int(length - 1);
		int da = int(a1 - a0) / int(length - 1);
		if (db < 0) { // make sure db is positive
			dr = -dr;
			db = -db;
			subRB = true;
		}
		if (da < 0) { // make sure da is positive
			dg = -dg;
			da = -da;
			subGA = true;
		}
		// also pack two 8.16 delta values in one [8.8 ; 8.8] vector
		drb = ((dr << 8) & 0xffff0000) |
		      ((db >> 8) & 0x0000ffff);
		dga = ((dg << 8) & 0xffff0000) |
		      ((da >> 8) & 0x0000ffff);
	}
}
// Pack two [8.8 ; 8.8] vectors into one pixel.
static unsigned packRGBA(unsigned rb, unsigned ga)
{
	return (rb & 0xff00ff00) | ((ga & 0xff00ff00) >> 8);
}

// Draw a gradient on the given surface. This is a bilinear interpolation
// between 4 RGBA colors. One color for each corner, in this order:
//    0 -- 1
//    |    |
//    2 -- 3
void gradient(const unsigned* rgba, SDL_Surface& surface, unsigned borderSize)
{
	int width  = surface.w - 2 * borderSize;
	int height = surface.h - 2 * borderSize;
	if ((width <= 0) || (height <= 0)) return;

	unsigned r0, g0, b0, a0;
	unsigned r1, g1, b1, a1;
	int dr02, dg02, db02, da02;
	int dr13, dg13, db13, da13;
	setupInterp1(rgba[0], rgba[2], height, r0, g0, b0, a0, dr02, dg02, db02, da02);
	setupInterp1(rgba[1], rgba[3], height, r1, g1, b1, a1, dr13, dg13, db13, da13);

	unsigned* buffer = static_cast<unsigned*>(surface.pixels);
	buffer += borderSize;
	buffer += borderSize * (surface.pitch / sizeof(unsigned));
	for (int y = 0; y < height; ++y) {
		unsigned  rb,  ga;
		unsigned drb, dga;
		bool   subRB, subGA;
		setupInterp2(r0, g0, b0, a0, r1, g1, b1, a1, width,
		             rb, ga, drb, dga, subRB, subGA);

		// Depending on the subRB/subGA booleans, we need to add or
		// subtract the delta to/from the initial value. There are
		// 2 booleans so 4 combinations:
		if (!subRB) {
			if (!subGA) {
				for (int x = 0; x < width; ++x) {
					buffer[x] = packRGBA(rb, ga);
					rb += drb; ga += dga;
				}
			} else {
				for (int x = 0; x < width; ++x) {
					buffer[x] = packRGBA(rb, ga);
					rb += drb; ga -= dga;
				}
			}
		} else {
			if (!subGA) {
				for (int x = 0; x < width; ++x) {
					buffer[x] = packRGBA(rb, ga);
					rb -= drb; ga += dga;
				}
			} else {
				for (int x = 0; x < width; ++x) {
					buffer[x] = packRGBA(rb, ga);
					rb -= drb; ga -= dga;
				}
			}
		}

		r0 += dr02; g0 += dg02; b0 += db02; a0 += da02;
		r1 += dr13; g1 += dg13; b1 += db13; a1 += da13;
		buffer += (surface.pitch / sizeof(unsigned));
	}
}

// class SDLImage

SDLImage::SDLImage(const string& filename)
	: image(loadImage(filename))
	, a(-1), flipX(false), flipY(false)
{
}

SDLImage::SDLImage(const std::string& filename, double scaleFactor)
	: image(loadImage(filename, scaleFactor))
	, a(-1), flipX(scaleFactor < 0), flipY(scaleFactor < 0)
{
}

SDLImage::SDLImage(const string& filename, int width, int height)
	: image(loadImage(filename, width, height))
	, a(-1), flipX(width < 0), flipY(height < 0)
{
}

SDLImage::SDLImage(int width, int height, unsigned rgba)
	: flipX(width < 0), flipY(height < 0)
{
	initSolid(width, height, rgba, 0, 0); // no border
}


SDLImage::SDLImage(int width, int height, const unsigned* rgba,
                   unsigned borderSize, unsigned borderRGBA)
	: flipX(width < 0), flipY(height < 0)
{
	if ((rgba[0] == rgba[1]) &&
	    (rgba[0] == rgba[2]) &&
	    (rgba[0] == rgba[3])) {
		initSolid   (width, height, rgba[0], borderSize, borderRGBA);
	} else {
		initGradient(width, height, rgba,    borderSize, borderRGBA);
	}
}

static unsigned convertColor(const SDL_PixelFormat& format, unsigned rgba)
{
	return SDL_MapRGBA(
#if SDL_VERSION_ATLEAST(1, 2, 12)
		&format,
#else
		// Work around const correctness bug in SDL 1.2.11 (bug #421).
		const_cast<SDL_PixelFormat*>(&format),
#endif
		(rgba >> 24) & 0xff,
		(rgba >> 16) & 0xff,
		(rgba >>  8) & 0xff,
		(rgba >>  0) & 0xff);
}

static void drawBorder(SDL_Surface& image, int size, unsigned rgba)
{
	if (size <= 0) return;

	unsigned color = convertColor(*image.format, rgba);
	bool onlyBorder = ((2 * size) >= image.w) ||
	                  ((2 * size) >= image.h);
	if (onlyBorder) {
		SDL_FillRect(&image, NULL, color);
	} else {
		// +--------------------+
		// |          1         |
		// +---+------------+---+
		// |   |            |   |
		// | 3 |            | 4 |
		// |   |            |   |
		// +---+------------+---+
		// |          2         |
		// +--------------------+
		SDL_Rect rect;
		rect.x = 0;
		rect.y = 0;
		rect.w = image.w;
		rect.h = size;
		SDL_FillRect(&image, &rect, color); // 1

		rect.y = image.h - size;
		SDL_FillRect(&image, &rect, color); // 2

		rect.y = size;
		rect.w = size;
		rect.h = image.h - 2 * size;
		SDL_FillRect(&image, &rect, color); // 3

		rect.x = image.w - size;
		SDL_FillRect(&image, &rect, color); // 4
	}
}

void SDLImage::initSolid(int width, int height, unsigned rgba,
                         unsigned borderSize, unsigned borderRGBA)
{
	checkSize(width, height);
	if ((width == 0) || (height == 0)) {
		// SDL_FillRect crashes on zero-width surfaces, so check for it
		return;
	}

	unsigned bgAlpha     = rgba       & 0xff;
	unsigned borderAlpha = borderRGBA & 0xff;
	if (bgAlpha == borderAlpha) {
		a = (bgAlpha == 255) ? 256 : bgAlpha;
	} else {
		a = -1;
	}

	// Figure out required bpp and color masks.
	Uint32 rmask, gmask, bmask, amask;
	unsigned bpp;
	if (a == -1) {
		// We need an alpha channel.
		//  The SDL documentation doesn't specify this, but I've
		//  checked the implemenation (SDL-1.2.15):
		//  SDL_DisplayFormatAlpha() always returns a 32bpp surface,
		//  also when the current display surface is 16bpp.
		bpp = 32;
		getRGBAmasks32(rmask, gmask, bmask, amask);
	} else {
		// No alpha channel, copy format of the display surface.
		SDL_Surface* videoSurface = SDL_GetVideoSurface();
		assert(videoSurface);
		const SDL_PixelFormat& format = *videoSurface->format;
		bpp   = format.BitsPerPixel;
		rmask = format.Rmask;
		gmask = format.Gmask;
		bmask = format.Bmask;
		amask = 0;
	}

	// Create surface with correct size/masks.
	image = SDLSurfacePtr(abs(width), abs(height), bpp,
	                      rmask, gmask, bmask, amask);

	// draw interior
	SDL_FillRect(image.get(), NULL, convertColor(*image->format, rgba));

	drawBorder(*image, borderSize, borderRGBA);
}

void SDLImage::initGradient(int width, int height, const unsigned* rgba_,
                            unsigned borderSize, unsigned borderRGBA)
{
	checkSize(width, height);
	if ((width == 0) || (height == 0)) {
		return;
	}

	unsigned rgba[4];
	for (unsigned i = 0; i < 4; ++i) {
		rgba[i] = rgba_[i];
	}

	if (((rgba[0] & 0xff) == (rgba[1] & 0xff)) &&
	    ((rgba[0] & 0xff) == (rgba[2] & 0xff)) &&
	    ((rgba[0] & 0xff) == (rgba[3] & 0xff)) &&
	    ((rgba[0] & 0xff) == (borderRGBA & 0xff))) {
		a = rgba[0] & 0xff;
	} else {
		a = -1;
	}

	if (flipX) {
		std::swap(rgba[0], rgba[1]);
		std::swap(rgba[2], rgba[3]);
	}
	if (flipY) {
		std::swap(rgba[0], rgba[2]);
		std::swap(rgba[1], rgba[3]);
	}

	SDLSurfacePtr tmp32 = create32BppSurface(width, height, a == -1);
	for (int i = 0; i < 4; ++i) {
		rgba[i] = convertColor(*tmp32->format, rgba[i]);
	}
	gradient(rgba, *tmp32, borderSize);
	drawBorder(*tmp32, borderSize, borderRGBA);

	SDL_PixelFormat& outFormat = *SDL_GetVideoSurface()->format;
	if ((outFormat.BitsPerPixel == 32) || (a == -1)) {
		if (outFormat.BitsPerPixel == 32) {
			// for 32bpp the format must match
			SDL_PixelFormat& inFormat  = *tmp32->format;
			(void)&inFormat;
			assert(inFormat.Rmask == outFormat.Rmask);
			assert(inFormat.Gmask == outFormat.Gmask);
			assert(inFormat.Bmask == outFormat.Bmask);
			// don't compare Amask
		} else {
			// For 16bpp with alpha channel, also create a 32bpp
			// image surface. See also comments in initSolid().
		}
		image = tmp32;
	} else {
		image.reset(SDL_DisplayFormat(tmp32.get()));
	}
}

SDLImage::SDLImage(SDLSurfacePtr image_)
	: image(image_)
	, a(-1), flipX(false), flipY(false)
{
}

void SDLImage::allocateWorkImage()
{
	int flags = SDL_SWSURFACE;
	if (PLATFORM_GP2X) {
		flags = SDL_HWSURFACE;
	}
	const SDL_PixelFormat* format = image->format;
	workImage.reset(SDL_CreateRGBSurface(flags,
		image->w, image->h, format->BitsPerPixel,
		format->Rmask, format->Gmask, format->Bmask, 0));
	if (!workImage.get()) {
		throw FatalError("Couldn't allocate SDLImage workimage");
	}
#if PLATFORM_GP2X
	GP2XMMUHack::instance().patchPageTables();
#endif
}

void SDLImage::draw(OutputSurface& output, int x, int y, byte alpha)
{
	if (!image.get()) return;
	if (flipX) x -= image->w;
	if (flipY) y -= image->h;

	output.unlock();
	SDL_Surface* outputSurface = output.getSDLWorkSurface();
	SDL_Rect rect;
	rect.x = x;
	rect.y = y;
	if (a == -1) {
		if (alpha == 255) {
			SDL_BlitSurface(image.get(), NULL, outputSurface, &rect);
		} else {
			if (!workImage.get()) {
				allocateWorkImage();
			}
			rect.w = image->w;
			rect.h = image->h;
			SDL_BlitSurface(outputSurface, &rect, workImage.get(), NULL);
			SDL_BlitSurface(image.get(),   NULL,  workImage.get(), NULL);
			SDL_SetAlpha(workImage.get(), SDL_SRCALPHA, alpha);
			SDL_BlitSurface(workImage.get(), NULL, outputSurface, &rect);
		}
	} else {
		SDL_SetAlpha(image.get(), SDL_SRCALPHA, (a * alpha) / 256);
		SDL_BlitSurface(image.get(), NULL, outputSurface, &rect);
	}
}

int SDLImage::getWidth() const
{
	return image.get() ? image->w : 0;
}

int SDLImage::getHeight() const
{
	return image.get() ? image->h : 0;
}

} // namespace openmsx
