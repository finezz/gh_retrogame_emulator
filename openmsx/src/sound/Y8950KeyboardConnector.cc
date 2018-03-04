// $Id: Y8950KeyboardConnector.cc 12629 2012-06-14 20:16:30Z m9710797 $

#include "Y8950KeyboardConnector.hh"
#include "Y8950KeyboardDevice.hh"
#include "DummyY8950KeyboardDevice.hh"
#include "checked_cast.hh"
#include "serialize.hh"

namespace openmsx {

Y8950KeyboardConnector::Y8950KeyboardConnector(
	PluggingController& pluggingController)
	: Connector(pluggingController, "audiokeyboardport",
	            std::auto_ptr<Pluggable>(new DummyY8950KeyboardDevice()))
	, data(255)
{
}

Y8950KeyboardConnector::~Y8950KeyboardConnector()
{
}

void Y8950KeyboardConnector::write(byte newData, EmuTime::param time)
{
	if (newData != data) {
		data = newData;
		getPluggedKeyb().write(data, time);
	}
}

byte Y8950KeyboardConnector::read(EmuTime::param time)
{
	return getPluggedKeyb().read(time);
}

const std::string Y8950KeyboardConnector::getDescription() const
{
	return "MSX-AUDIO keyboard connector";
}

string_ref Y8950KeyboardConnector::getClass() const
{
	return "Y8950 Keyboard Port";
}

void Y8950KeyboardConnector::plug(Pluggable& dev, EmuTime::param time)
{
	Connector::plug(dev, time);
	getPluggedKeyb().write(data, time);
}

Y8950KeyboardDevice& Y8950KeyboardConnector::getPluggedKeyb() const
{
	return *checked_cast<Y8950KeyboardDevice*>(&getPlugged());
}

template<typename Archive>
void Y8950KeyboardConnector::serialize(Archive& ar, unsigned /*version*/)
{
	ar.template serializeBase<Connector>(*this);
	// don't serialize 'data', done in Y8950
}
INSTANTIATE_SERIALIZE_METHODS(Y8950KeyboardConnector);

} // namespace openmsx
