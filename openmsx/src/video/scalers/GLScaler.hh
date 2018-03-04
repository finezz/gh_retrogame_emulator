// $Id: GLScaler.hh 12675 2012-06-25 18:26:55Z m9710797 $

#ifndef GLSCALER_HH
#define GLSCALER_HH

#include <memory>

namespace openmsx {

class ColorTexture;
class FrameSource;
class ShaderProgram;

/** Abstract base class for OpenGL scalers.
  * A scaler is an algorithm that converts low-res graphics to hi-res graphics.
  */
class GLScaler
{
public:
	GLScaler();
	virtual ~GLScaler();

	/** Scales the image in the given area, which must consist of lines which
	  * are all equally wide.
	  * Scaling factor depends on the concrete scaler.
	  * @param src Source: texture containing the frame to be scaled.
	  * @param superImpose Texture containing the to-be-superimposed image (can be NULL).
	  * @param srcStartY Y-coordinate of the top source line (inclusive).
	  * @param srcEndY Y-coordinate of the bottom source line (exclusive).
	  * @param srcWidth The number of pixels per line for the given area.
	  * @param dstStartY Y-coordinate of the top destination line (inclusive).
	  * @param dstEndY Y-coordinate of the bottom destination line (exclusive).
	  * @param dstWidth The number of pixels per line on the output screen.
	  * @param logSrcHeight The logical height of the complete src texture
	  *        (actual texture height can be double as high in case of
	  *        non-interlace). This is needed to translate src-Y-coordinates
	  *        to superImpose-Y-coordinates.
	  */
	virtual void scaleImage(
		ColorTexture& src, ColorTexture* superImpose,
		unsigned srcStartY, unsigned srcEndY, unsigned srcWidth,
		unsigned dstStartY, unsigned dstEndY, unsigned dstWidth,
		unsigned logSrcHeight) = 0;

	virtual void uploadBlock(
		unsigned srcStartY, unsigned srcEndY,
		unsigned lineWidth, FrameSource& paintFrame);

protected:
	/** Helper method to draw a rectangle with multiple texture coordinates.
	  * This method is similar to Texture::drawRect() but it calculates a
	  * seconds set of texture coordinates. The first tex-coords are used
	  * for the MSX texture (textur unit 0), and are calculated from
	  * src{Start,End}Y and physSrcHeight. The second set of tex-coord are
	  * used for the superimpose texture (texture unit 1) and are calculated
	  * from src{Start,End}Y and logSrcHeight.
	  * @param src
	  * @param srcStartY
	  * @param srcEndY
	  * @param physSrcHeight
	  * @param logSrcHeight
	  * @param dstStartY
	  * @param dstEndY
	  * @param dstWidth
	  * @param textureFromZero If true, the texture coordinates of subpixels
	  *   will start from zero: for example in 4x zoom the source coordinates
	  *   will be 0.0, 0.25, 0.5, 0.75. If false, the texture coordinates of
	  *   subpixels will be centered: for example in 4x zoom the source
	  *   coordinates will be 0.125, 0.375, 0.625, 0.875.
	  */
	void drawMultiTex(ColorTexture& src,
	                  unsigned srcStartY, unsigned srcEndY,
	                  float physSrcHeight, float logSrcHeight,
	                  unsigned dstStartY, unsigned dstEndY, unsigned dstWidth,
	                  bool textureFromZero = false);

private:
	std::auto_ptr<ShaderProgram> scalerProgram;
};

} // namespace openmsx

#endif // GLSCALER_HH
