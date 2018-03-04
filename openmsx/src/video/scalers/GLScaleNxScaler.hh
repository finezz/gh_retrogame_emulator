// $Id: GLScaleNxScaler.hh 11789 2010-12-04 17:05:22Z mthuurne $

#ifndef GLSCALENXSCALER_HH
#define GLSCALENXSCALER_HH

#include "GLScaler.hh"
#include "GLUtil.hh"
#include "noncopyable.hh"
#include <memory>

namespace openmsx {

class ShaderProgram;

class GLScaleNxScaler : public GLScaler, private noncopyable
{
public:
	GLScaleNxScaler();

	virtual void scaleImage(
		ColorTexture& src, ColorTexture* superImpose,
		unsigned srcStartY, unsigned srcEndY, unsigned srcWidth,
		unsigned dstStartY, unsigned dstEndY, unsigned dstWidth,
		unsigned logSrcHeight);

private:
	std::auto_ptr<ShaderProgram> scalerProgram[2];
	GLint texSizeLoc[2];
};

} // namespace openmsx

#endif // GLSCALENXSCALER_HH
