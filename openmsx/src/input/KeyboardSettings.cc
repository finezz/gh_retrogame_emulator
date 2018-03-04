// $Id: KeyboardSettings.cc 11866 2011-01-03 19:07:53Z m9710797 $

#include "KeyboardSettings.hh"
#include "EnumSetting.hh"
#include "BooleanSetting.hh"
#include <cassert>

namespace openmsx {

KeyboardSettings::KeyboardSettings(CommandController& commandController)
	: alwaysEnableKeypad(new BooleanSetting(commandController,
		"kbd_numkeypad_always_enabled",
		"Numeric keypad is always enabled, even on an MSX that does not have one",
		false))
	, traceKeyPresses(new BooleanSetting(commandController,
		"kbd_trace_key_presses",
		"Trace key presses (show SDL key code, SDL modifiers and Unicode code-point value)",
		false, Setting::DONT_SAVE))
	, autoToggleCodeKanaLock(new BooleanSetting(commandController,
		"kbd_auto_toggle_code_kana_lock",
		"Automatically toggle the CODE/KANA lock, based on the characters entered on the host keyboard",
		true))
{
	EnumSetting<Keys::KeyCode>::Map allowedKeys;
	allowedKeys["RALT"]        = Keys::K_RALT;
	allowedKeys["MENU"]        = Keys::K_MENU;
	allowedKeys["RCTRL"]       = Keys::K_RCTRL;
	allowedKeys["HENKAN_MODE"] = Keys::K_HENKAN_MODE;
	allowedKeys["RSHIFT"]      = Keys::K_RSHIFT;
	allowedKeys["RMETA"]       = Keys::K_RMETA;
	allowedKeys["LMETA"]       = Keys::K_LMETA;
	allowedKeys["LSUPER"]      = Keys::K_LSUPER;
	allowedKeys["RSUPER"]      = Keys::K_RSUPER;
	allowedKeys["HELP"]        = Keys::K_HELP;
	allowedKeys["UNDO"]        = Keys::K_UNDO;
	allowedKeys["END"]         = Keys::K_END;
	allowedKeys["PAGEUP"]      = Keys::K_PAGEUP;
	allowedKeys["PAGEDOWN"]    = Keys::K_PAGEDOWN;
	codeKanaHostKey.reset(new EnumSetting<Keys::KeyCode>(
		commandController, "kbd_code_kana_host_key",
		"Host key that maps to the MSX CODE/KANA key. Please note that the HENKAN_MODE key only exists on Japanese host keyboards)",
		Keys::K_RALT, allowedKeys));

	deadkeyHostKey[0].reset(new EnumSetting<Keys::KeyCode>(
		commandController, "kbd_deadkey1_host_key",
		"Host key that maps to deadkey 1. Not applicable to Japanese and Korean MSX models",
		Keys::K_RCTRL, allowedKeys));

	deadkeyHostKey[1].reset(new EnumSetting<Keys::KeyCode>(
		commandController, "kbd_deadkey2_host_key",
		"Host key that maps to deadkey 2. Only applicable to Brazilian MSX models (Sharp Hotbit and Gradiente)",
		Keys::K_PAGEUP, allowedKeys));

	deadkeyHostKey[2].reset(new EnumSetting<Keys::KeyCode>(
		commandController, "kbd_deadkey3_host_key",
		"Host key that maps to deadkey 3. Only applicable to Brazilian Sharp Hotbit MSX models",
		Keys::K_PAGEDOWN, allowedKeys));

	EnumSetting<KpEnterMode>::Map kpEnterModeMap;
	kpEnterModeMap["KEYPAD_COMMA"] = MSX_KP_COMMA;
	kpEnterModeMap["ENTER"] = MSX_ENTER;
	kpEnterMode.reset(new EnumSetting<KpEnterMode>(
		commandController, "kbd_numkeypad_enter_key",
		"MSX key that the enter key on the host numeric keypad must map to",
		MSX_KP_COMMA, kpEnterModeMap));

	EnumSetting<MappingMode>::Map mappingModeMap;
	mappingModeMap["KEY"] = KEY_MAPPING;
	mappingModeMap["CHARACTER"] = CHARACTER_MAPPING;
	mappingMode.reset(new EnumSetting<MappingMode>(
		commandController, "kbd_mapping_mode",
		"Keyboard mapping mode",
		CHARACTER_MAPPING, mappingModeMap));
}

KeyboardSettings::~KeyboardSettings()
{
}

EnumSetting<Keys::KeyCode>& KeyboardSettings::getCodeKanaHostKey() const
{
	return *codeKanaHostKey;
}

Keys::KeyCode KeyboardSettings::getDeadkeyHostKey(unsigned n) const
{
	assert(n < 3);
	return deadkeyHostKey[n]->getValue();
}

EnumSetting<KeyboardSettings::KpEnterMode>& KeyboardSettings::getKpEnterMode() const
{
	return *kpEnterMode;
}

EnumSetting<KeyboardSettings::MappingMode>& KeyboardSettings::getMappingMode() const
{
	return *mappingMode;
}

BooleanSetting& KeyboardSettings::getAlwaysEnableKeypad() const
{
	return *alwaysEnableKeypad;
}

BooleanSetting& KeyboardSettings::getTraceKeyPresses() const
{
	return *traceKeyPresses;
}

BooleanSetting& KeyboardSettings::getAutoToggleCodeKanaLock() const
{
	return *autoToggleCodeKanaLock;
}

} // namespace openmsx
