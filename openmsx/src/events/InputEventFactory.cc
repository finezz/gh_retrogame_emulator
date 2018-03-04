// $Id: InputEventFactory.cc 12626 2012-06-14 20:14:17Z m9710797 $

#include "InputEventFactory.hh"
#include "InputEvents.hh"
#include "CommandException.hh"
#include "StringOp.hh"
#include "Interpreter.hh"

using std::string;
using std::vector;

namespace openmsx {

namespace InputEventFactory {

static EventPtr parseKeyEvent(const string& str, const int unicode)
{
	Keys::KeyCode keyCode = Keys::getCode(str);
	if (keyCode == Keys::K_NONE) {
		throw CommandException("Invalid keycode: " + str);
	}
	if (keyCode & Keys::KD_RELEASE) {
		return EventPtr(new KeyUpEvent(keyCode, unicode));
	} else {
		return EventPtr(new KeyDownEvent(keyCode, unicode));
	}
}

static EventPtr parseKeyEvent(
		const string& str, const vector<string>& components)
{
	if (components.size() == 2) {
		return parseKeyEvent(components[1], 0);
	} else if ((components.size() == 3) &&
	           (StringOp::startsWith(components[2], "unicode"))) {
		return parseKeyEvent(components[1],
		                     stoi(string_ref(components[2]).substr(7)));
	} else {
		throw CommandException("Invalid keyboard event: " + str);
	}
}

static bool upDown(const string& str)
{
	if (str == "up") {
		return true;
	} else if (str == "down") {
		return false;
	} else {
		throw CommandException("Invalid direction (expected 'up' or 'down'): " + str);
	}
}

static EventPtr parseMouseEvent(
		const string& str, const vector<string>& components)
{
	if (components.size() < 2) {
		throw CommandException("Invalid mouse event: " + str);
	}
	if (components[1] == "motion") {
		if (components.size() != 4) {
			throw CommandException("Invalid mouse motion event: " + str);
		}
		return EventPtr(new MouseMotionEvent(
			StringOp::stringToInt(components[2]),
			StringOp::stringToInt(components[3])));
	} else if (StringOp::startsWith(components[1], "button")) {
		if (components.size() != 3) {
			throw CommandException("Invalid mouse button event: " + str);
		}
		unsigned button = stoi(string_ref(components[1]).substr(6));
		if (upDown(components[2])) {
			return EventPtr(new MouseButtonUpEvent(button));
		} else {
			return EventPtr(new MouseButtonDownEvent(button));
		}
	} else {
		throw CommandException("Invalid mouse event: " + str);
	}
}

static EventPtr parseJoystickEvent(
		const string& str, const vector<string>& components)
{
	if (components.size() != 3) {
		throw CommandException("Invalid joystick event: " + str);
	}
	int joystick;
	string joyString = components[0].substr(3);
	if (!StringOp::stringToInt(joyString, joystick) || (joystick == 0)) {
		throw CommandException("Invalid joystick number: " + joyString);
	}
	--joystick;

	if (StringOp::startsWith(components[1], "button")) {
		int button;
		string joyButtonString = components[1].substr(6);
		if (!StringOp::stringToInt(joyButtonString, button)) {
			throw CommandException("Invalid joystick button number: " + joyButtonString);
		}
		if (upDown(components[2])) {
			return EventPtr(new JoystickButtonUpEvent(joystick, button));
		} else {
			return EventPtr(new JoystickButtonDownEvent(joystick, button));
		}
	} else if (StringOp::startsWith(components[1], "axis")) {
		int axis;
		string axisString = components[1].substr(4);
		if (!StringOp::stringToInt(axisString, axis)) {
			throw CommandException("Invalid axis number: " + axisString);
		}
		int value;
		if (!StringOp::stringToInt(components[2], value) || (short(value) != value)) {
			throw CommandException("Invalid value: " + components[2]);
		}
		return EventPtr(new JoystickAxisMotionEvent(joystick, axis, value));
	} else {
		throw CommandException("Invalid joystick event: " + str);
	}
}

static EventPtr parseFocusEvent(
		const string& str, const vector<string>& components)
{
	if (components.size() != 2) {
		throw CommandException("Invalid focus event: " + str);
	}
	bool gain = StringOp::stringToBool(components[1]);
	return EventPtr(new FocusEvent(gain));
}

static EventPtr parseResizeEvent(
		const string& str, const vector<string>& components)
{
	if (components.size() != 3) {
		throw CommandException("Invalid resize event: " + str);
	}
	int x = StringOp::stringToInt(components[1]);
	int y = StringOp::stringToInt(components[2]);
	return EventPtr(new ResizeEvent(x, y));
}

static EventPtr parseQuitEvent(
		const string& str, const vector<string>& components)
{
	if (components.size() != 1) {
		throw CommandException("Invalid quit event: " + str);
	}
	return EventPtr(new QuitEvent());
}

EventPtr createInputEvent(const string& str)
{
	vector<string> components;
	Interpreter::splitList(str, components, 0);
	if (components.empty()) {
		throw CommandException("Invalid event: \"" + str + '\"');
	}
	if (components[0] == "keyb") {
		return parseKeyEvent(str, components);
	} else if (components[0] == "mouse") {
		return parseMouseEvent(str, components);
	} else if (StringOp::startsWith(components[0], "joy")) {
		return parseJoystickEvent(str, components);
	} else if (components[0] == "focus") {
		return parseFocusEvent(str, components);
	} else if (components[0] == "resize") {
		return parseResizeEvent(str, components);
	} else if (components[0] == "quit") {
		return parseQuitEvent(str, components);
	} else if (components[0] == "command") {
		return EventPtr();
		//return parseCommandEvent(str, components);
	} else {
		// fall back
		return parseKeyEvent(components[0], 0);
	}
}

} // namespace InputEventFactory

} // namespace openmsx
