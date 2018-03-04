// $Id: Connector.cc 12630 2012-06-14 20:17:01Z m9710797 $

#include "Connector.hh"
#include "Pluggable.hh"
#include "PluggingController.hh"
#include "serialize.hh"
#include "CliComm.hh"

namespace openmsx {

Connector::Connector(PluggingController& pluggingController_,
                     string_ref name_, std::auto_ptr<Pluggable> dummy_)
	: pluggingController(pluggingController_)
	, name(name_.data(), name_.size())
	, dummy(dummy_)
{
	plugged = dummy.get();
	pluggingController.registerConnector(*this);
}

Connector::~Connector()
{
	pluggingController.unregisterConnector(*this);
}

const std::string& Connector::getName() const
{
	return name;
}

void Connector::plug(Pluggable& device, EmuTime::param time)
{
	device.plug(*this, time);
	plugged = &device; // not executed if plug fails
}

void Connector::unplug(EmuTime::param time)
{
	plugged->unplug(time);
	plugged = dummy.get();
}

Pluggable& Connector::getPlugged() const
{
	return *plugged;
}

template<typename Archive>
void Connector::serialize(Archive& ar, unsigned /*version*/)
{
	std::string plugName;
	if (!ar.isLoader() && (plugged != dummy.get())) {
		plugName = plugged->getName();
	}
	ar.serialize("plugName", plugName);

	if (!ar.isLoader()) {
		if (!plugName.empty()) {
			ar.beginSection();
			ar.serializePolymorphic("pluggable", *plugged);
			ar.endSection();
		}
	} else {
		if (plugName.empty()) {
			// was not plugged in
			plugged = dummy.get();
		} else if (Pluggable* pluggable =
			       pluggingController.findPluggable(plugName)) {
			plugged = pluggable;
			// set connector before loading the pluggable so that
			// the pluggable can test whether it was connected
			pluggable->setConnector(this);
			ar.skipSection(false);
			ar.serializePolymorphic("pluggable", *plugged);
		} else {
			// was plugged, but we don't have that pluggable anymore
			pluggingController.getCliComm().printWarning(
				"Pluggable \"" + plugName + "\" was plugged in, "
				"but is not available anymore on this system, "
				"so it will be ignored.");
			ar.skipSection(true);
			plugged = dummy.get();
		}
	}
}
INSTANTIATE_SERIALIZE_METHODS(Connector);

} // namespace openmsx
