// $Id: Mouse.hh 12631 2012-06-14 20:18:24Z m9710797 $

#ifndef MOUSE_HH
#define MOUSE_HH

#include "JoystickDevice.hh"
#include "MSXEventListener.hh"
#include "StateChangeListener.hh"
#include "Clock.hh"
#include "serialize_meta.hh"

namespace openmsx {

class MSXEventDistributor;
class StateChangeDistributor;

class Mouse : public JoystickDevice, private MSXEventListener
            , private StateChangeListener
{
public:
	Mouse(MSXEventDistributor& eventDistributor,
	      StateChangeDistributor& stateChangeDistributor);
	virtual ~Mouse();

	template<typename Archive>
	void serialize(Archive& ar, unsigned version);

private:
	// Pluggable
	virtual const std::string& getName() const;
	virtual string_ref getDescription() const;
	virtual void plugHelper(Connector& connector, EmuTime::param time);
	virtual void unplugHelper(EmuTime::param time);

	// JoystickDevice
	virtual byte read(EmuTime::param time);
	virtual void write(byte value, EmuTime::param time);

	// MSXEventListener
	virtual void signalEvent(const shared_ptr<const Event>& event,
	                         EmuTime::param time);
	// StateChangeListener
	virtual void signalStateChange(const shared_ptr<StateChange>& event);
	virtual void stopReplay(EmuTime::param time);

	void createMouseStateChange(EmuTime::param time,
		int deltaX, int deltaY, byte press, byte release);
	void emulateJoystick();
	void plugHelper2();

	MSXEventDistributor& eventDistributor;
	StateChangeDistributor& stateChangeDistributor;
	Clock<1000> lastTime; // ms
	int faze;
	int xrel, yrel;
	int curxrel, curyrel;
	int absHostX, absHostY;
	byte status;
	bool mouseMode;
};
SERIALIZE_CLASS_VERSION(Mouse, 3);

} // namespace openmsx

#endif
