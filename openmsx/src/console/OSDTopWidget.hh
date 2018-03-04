// $Id: OSDTopWidget.hh 12620 2012-06-14 20:10:19Z m9710797 $

#ifndef OSDTOPWIDGET_HH
#define OSDTOPWIDGET_HH

#include "OSDWidget.hh"

namespace openmsx {

class OSDTopWidget : public OSDWidget
{
public:
	OSDTopWidget();
	virtual string_ref getType() const;
	virtual void getWidthHeight(const OutputRectangle& output,
	                            double& width, double& height) const;

protected:
	virtual void invalidateLocal();
	virtual void paintSDL(OutputSurface& output);
	virtual void paintGL (OutputSurface& output);
};

} // namespace openmsx

#endif
