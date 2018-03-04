// $Id: Scanline.hh 12117 2011-04-16 17:52:48Z m9710797 $

#ifndef SCANLINE_HH
#define SCANLINE_HH

#include "PixelOperations.hh"
#include "openmsx.hh"

namespace openmsx {

/**
 * Helper class to perform 'pixel x scalar' calculations.
 */
template<typename Pixel> class Multiply;

template<> class Multiply<word>
{
public:
	explicit Multiply(const PixelOperations<word>& pixelOps);
	void setFactor(unsigned f);
	inline word multiply(word p, unsigned factor) const;
	inline word multiply(word p) const;
	inline const word* getTable() const;
private:
	const PixelOperations<word>& pixelOps;
	unsigned factor;
	word tab[0x10000];
};

template<> class Multiply<unsigned>
{
public:
	explicit Multiply(const PixelOperations<unsigned>& pixelOps);
	void setFactor(unsigned f);
	inline unsigned multiply(unsigned p, unsigned factor) const;
	inline unsigned multiply(unsigned p) const;
	const unsigned* getTable() const;
private:
	unsigned factor;
};

/**
 * Helper class to draw scalines
 */
template <class Pixel> class Scanline
{
public:
	explicit Scanline(const PixelOperations<Pixel>& pixelOps);

	/** Draws a scanline. The scanline will be the average of the two
	  * input lines and darkened by a certain factor.
	  * @param src1 First input line.
	  * @param src2 Second input line.
	  * @param dst Output line.
	  * @param factor Darkness factor, 0 means completely black,
	  *               255 means no darkening.
	  * @param width Line width in pixels.
	  */
	void draw(const Pixel* src1, const Pixel* src2, Pixel* dst,
	          unsigned factor, unsigned long width);

	/** Darken one pixel. Typically used to implement drawBlank().
	 */
	Pixel darken(Pixel p, unsigned factor);

	/** Darken and blend two pixels.
	 */
	Pixel darken(Pixel p1, Pixel p2, unsigned factor);

private:
	Multiply<Pixel> darkener;
	PixelOperations<Pixel> pixelOps;
};

} // namespace openmsx

#endif
