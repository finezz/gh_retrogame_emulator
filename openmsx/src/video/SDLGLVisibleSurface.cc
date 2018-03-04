// $Id: SDLGLVisibleSurface.cc 11381 2010-04-04 17:02:06Z mthuurne $

#include "SDLGLVisibleSurface.hh"
#include "SDLGLOffScreenSurface.hh"
#include "GLSnow.hh"
#include "OSDConsoleRenderer.hh"
#include "OSDGUILayer.hh"
#include "InitException.hh"

namespace openmsx {

SDLGLVisibleSurface::SDLGLVisibleSurface(
		unsigned width, unsigned height, bool fullscreen,
		RenderSettings& renderSettings,
		EventDistributor& eventDistributor,
		InputEventGenerator& inputEventGenerator,
		FrameBuffer frameBuffer)
	: VisibleSurface(renderSettings, eventDistributor, inputEventGenerator)
	, SDLGLOutputSurface(frameBuffer)
{
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
	int flags = SDL_OPENGL | SDL_HWSURFACE | SDL_DOUBLEBUF |
	            (fullscreen ? SDL_FULLSCREEN : 0);
	//flags |= SDL_RESIZABLE;
	createSurface(width, height, flags);

	// The created surface may be larger than requested.
	// If that happens, center the area that we actually use.
	SDL_Surface* surface = getSDLDisplaySurface();
	unsigned actualWidth  = surface->w;
	unsigned actualHeight = surface->h;
	surface->w = width;
	surface->h = height;
	setPosition((actualWidth - width ) / 2, (actualHeight - height) / 2);

	// From the glew documentation:
	//   GLEW obtains information on the supported extensions from the
	//   graphics driver. Experimental or pre-release drivers, however,
	//   might not report every available extension through the standard
	//   mechanism, in which case GLEW will report it unsupported. To
	//   circumvent this situation, the glewExperimental global switch can
	//   be turned on by setting it to GL_TRUE before calling glewInit(),
	//   which ensures that all extensions with valid entry points will be
	//   exposed.
	// The 'glewinfo' utility also sets this flag before reporting results,
	// so I believe it would cause less confusion to do the same here.
	glewExperimental = GL_TRUE;

	// Initialise GLEW library.
	// This must happen after GL itself is initialised, which is done by
	// the SDL_SetVideoMode() call in createSurface().
	GLenum glew_error = glewInit();
	if (glew_error != GLEW_OK) {
		throw InitException(
			"Failed to init GLEW: " + std::string(
				reinterpret_cast<const char*>(
					glewGetErrorString(glew_error))));
	}

	glViewport(getX(), getY(), width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width, height, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);

	// This stuff logically belongs in the SDLGLOutputSurface constructor,
	// but it cannot be executed before the openGL context is created which
	// is done in this constructor. So construction of SDLGLOutputSurface
	// is split in two phases.
	SDLGLOutputSurface::init(*this);
}

SDLGLVisibleSurface::~SDLGLVisibleSurface()
{
}

void SDLGLVisibleSurface::flushFrameBuffer()
{
	SDLGLOutputSurface::flushFrameBuffer(getWidth(), getHeight());
}

void SDLGLVisibleSurface::saveScreenshot(const std::string& filename)
{
	SDLGLOutputSurface::saveScreenshot(filename, getWidth(), getHeight());
}

void SDLGLVisibleSurface::finish()
{
	SDL_GL_SwapBuffers();
}

std::auto_ptr<Layer> SDLGLVisibleSurface::createSnowLayer(Display& display)
{
	return std::auto_ptr<Layer>(new GLSnow(display, getWidth(), getHeight()));
}

std::auto_ptr<Layer> SDLGLVisibleSurface::createConsoleLayer(
		Reactor& reactor, CommandConsole& console)
{
	const bool openGL = true;
	return std::auto_ptr<Layer>(new OSDConsoleRenderer(
		reactor, console, getWidth(), getHeight(), openGL));
}

std::auto_ptr<Layer> SDLGLVisibleSurface::createOSDGUILayer(OSDGUI& gui)
{
	return std::auto_ptr<Layer>(new GLOSDGUILayer(gui));
}

std::auto_ptr<OutputSurface> SDLGLVisibleSurface::createOffScreenSurface()
{
	return std::auto_ptr<OutputSurface>(new SDLGLOffScreenSurface(*this));
}

} // namespace openmsx
