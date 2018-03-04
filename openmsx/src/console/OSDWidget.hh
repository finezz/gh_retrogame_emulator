// $Id: OSDWidget.hh 12620 2012-06-14 20:10:19Z m9710797 $

#ifndef OSDWIDGET_HH
#define OSDWIDGET_HH

#include "StringMap.hh"
#include "shared_ptr.hh"
#include "string_ref.hh"
#include <vector>
#include <memory>
#include <set>

namespace openmsx {

class OutputRectangle;
class OutputSurface;
class TclObject;

class OSDWidget
{
public:
	virtual ~OSDWidget();

	const std::string& getName() const;
	double getX()    const { return x; }
	double getY()    const { return y; }
	double getZ()    const { return z; }
	double getRelX() const { return relx; }
	double getRelY() const { return rely; }

	OSDWidget* getParent();
	const OSDWidget* getParent() const;
	OSDWidget* findSubWidget(string_ref name);
	const OSDWidget* findSubWidget(string_ref name) const;
	void addWidget(const shared_ptr<OSDWidget>& widget);
	void deleteWidget(OSDWidget& widget);

	virtual void getProperties(std::set<std::string>& result) const;
	virtual void setProperty(string_ref name, const TclObject& value);
	virtual void getProperty(string_ref name, TclObject& result) const;
	virtual double getRecursiveFadeValue() const;
	virtual string_ref getType() const = 0;

	void invalidateRecursive();
	void paintSDLRecursive(OutputSurface& output);
	void paintGLRecursive (OutputSurface& output);

	int getScaleFactor(const OutputRectangle& surface) const;
	void transformXY(const OutputRectangle& output,
	                 double x, double y, double relx, double rely,
	                 double& outx, double& outy) const;
	void getBoundingBox(const OutputRectangle& output,
	                    int& x, int& y, int& w, int& h);
	virtual void getWidthHeight(const OutputRectangle& output,
	                            double& width, double& height) const = 0;

protected:
	explicit OSDWidget(const std::string& name);
	void invalidateChildren();
	bool needSuppressErrors() const;

	virtual void invalidateLocal() = 0;
	virtual void paintSDL(OutputSurface& output) = 0;
	virtual void paintGL (OutputSurface& output) = 0;

private:
	void getMouseCoord(double& outx, double& outy) const;
	void transformReverse(const OutputRectangle& output,
	                      double x, double y,
	                      double& outx, double& outy) const;
	void setParent(OSDWidget* parent);
	void resortUp  (OSDWidget* elem);
	void resortDown(OSDWidget* elem);

	void listWidgetNames(const std::string& parentName,
	                     std::set<std::string>& result) const;
	friend class OSDCommand;

	// note: must be shared_ptr (not unique_ptr), see OSDWidget::paintSDLRecursive()
	typedef std::vector<shared_ptr<OSDWidget> > SubWidgets;
	typedef StringMap<OSDWidget*> SubWidgetsMap;

	/** Direct child widgets of this widget, sorted by z-coordinate.
	  */
	SubWidgets subWidgets;

	/** Contains the same widgets as "subWidgets", but stored with their name
	  * the key, so lookup by name is fast.
	  */
	SubWidgetsMap subWidgetsMap;

	OSDWidget* parent;

	const std::string name;
	double x, y, z;
	double relx, rely;
	bool scaled;
	bool clip;
	bool suppressErrors;
};

} // namespace openmsx

#endif
