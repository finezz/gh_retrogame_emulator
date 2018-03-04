// $Id: OSDGUI.hh 11817 2010-12-13 20:59:06Z m9710797 $

#ifndef OSDGUI_HH
#define OSDGUI_HH

#include "noncopyable.hh"
#include <memory>

namespace openmsx {

class OSDWidget;
class OSDCommand;
class Display;
class CommandController;

class OSDGUI : private noncopyable
{
public:
	OSDGUI(CommandController& commandController, Display& display);
	~OSDGUI();

	Display& getDisplay() const;
	OSDWidget& getTopWidget() const;
	void refresh() const;

	void setOpenGL(bool openGL_) { openGL = openGL_; }
	bool isOpenGL() const { return openGL; };

private:
	Display& display;
	const std::auto_ptr<OSDCommand> osdCommand;
	const std::auto_ptr<OSDWidget> topWidget;
	bool openGL;
};

} // namespace openmsx

#endif
