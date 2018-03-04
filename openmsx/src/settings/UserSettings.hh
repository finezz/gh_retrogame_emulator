// $Id: UserSettings.hh 12618 2012-06-14 20:09:25Z m9710797 $

#ifndef USERSETTINGS_HH
#define USERSETTINGS_HH

#include "noncopyable.hh"
#include "string_ref.hh"
#include <vector>
#include <memory>

namespace openmsx {

class CommandController;
class UserSettingCommand;
class Setting;

class UserSettings : private noncopyable
{
public:
	typedef std::vector<Setting*> Settings;

	explicit UserSettings(CommandController& commandController);
	~UserSettings();

	void addSetting(std::auto_ptr<Setting> setting);
	void deleteSetting(Setting& setting);
	Setting* findSetting(string_ref name) const;
	const Settings& getSettings() const;

private:
	const std::auto_ptr<UserSettingCommand> userSettingCommand;
	Settings settings;
};

} // namespace openmsx

#endif
