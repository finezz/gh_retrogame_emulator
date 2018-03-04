// $Id: SettingsConfig.cc 12609 2012-06-12 21:07:44Z m9710797 $

#include "SettingsConfig.hh"
#include "SettingsManager.hh"
#include "XMLElement.hh"
#include "XMLLoader.hh"
#include "LocalFileReference.hh"
#include "File.hh"
#include "FileContext.hh"
#include "FileException.hh"
#include "CliComm.hh"
#include "HotKey.hh"
#include "CommandException.hh"
#include "GlobalCommandController.hh"
#include "Command.hh"
#include "openmsx.hh"
#include <memory>
#include <cassert>

using std::auto_ptr;
using std::string;
using std::vector;

namespace openmsx {

class SaveSettingsCommand : public Command
{
public:
	SaveSettingsCommand(CommandController& commandController,
			    SettingsConfig& settingsConfig);
	virtual string execute(const vector<string>& tokens);
	virtual string help   (const vector<string>& tokens) const;
	virtual void tabCompletion(vector<string>& tokens) const;
private:
	SettingsConfig& settingsConfig;
};

class LoadSettingsCommand : public Command
{
public:
	LoadSettingsCommand(CommandController& commandController,
			    SettingsConfig& settingsConfig);
	virtual string execute(const vector<string>& tokens);
	virtual string help   (const vector<string>& tokens) const;
	virtual void tabCompletion(vector<string>& tokens) const;
private:
	SettingsConfig& settingsConfig;
};


SettingsConfig::SettingsConfig(
		GlobalCommandController& globalCommandController,
		HotKey& hotKey_)
	: commandController(globalCommandController)
	, saveSettingsCommand(new SaveSettingsCommand(commandController, *this))
	, loadSettingsCommand(new LoadSettingsCommand(commandController, *this))
	, settingsManager(new SettingsManager(globalCommandController))
	, xmlElement(new XMLElement("settings"))
	, hotKey(hotKey_)
	, mustSaveSettings(false)
{
}

SettingsConfig::~SettingsConfig()
{
	PRT_DEBUG("~SettingsConfig...");
	if (mustSaveSettings) {
		try {
			PRT_DEBUG("Saving Settings...");
			saveSetting();
			PRT_DEBUG("Saving Settings... DONE");
		} catch (FileException& e) {
			commandController.getCliComm().printWarning(
				"Auto-saving of settings failed: " + e.getMessage() );
		}
	}
}

void SettingsConfig::loadSetting(FileContext& context, const string& filename)
{
	LocalFileReference file(context.resolve(filename));
	xmlElement = XMLLoader::load(file.getFilename(), "settings.dtd");
	getSettingsManager().loadSettings(*xmlElement);
	hotKey.loadBindings(*xmlElement);

	// only set saveName after file was successfully parsed
	setSaveFilename(context, filename);
}

void SettingsConfig::setSaveFilename(FileContext& context, const string& filename)
{
	saveName = context.resolveCreate(filename);
}

void SettingsConfig::saveSetting(const string& filename)
{
	const string& name = filename.empty() ? saveName : filename;
	if (name.empty()) return;

	getSettingsManager().saveSettings(*xmlElement);
	hotKey.saveBindings(*xmlElement);

	File file(name, File::TRUNCATE);
	string data = "<!DOCTYPE settings SYSTEM 'settings.dtd'>\n" +
	              xmlElement->dump();
	file.write(data.data(), unsigned(data.size()));
}

void SettingsConfig::setSaveSettings(bool save)
{
	mustSaveSettings = save;
}

SettingsManager& SettingsConfig::getSettingsManager()
{
	return *settingsManager; // ***
}

XMLElement& SettingsConfig::getXMLElement()
{
	assert(xmlElement.get());
	return *xmlElement;
}


// class SaveSettingsCommand

SaveSettingsCommand::SaveSettingsCommand(
		CommandController& commandController,
		SettingsConfig& settingsConfig_)
	: Command(commandController, "save_settings")
	, settingsConfig(settingsConfig_)
{
}

string SaveSettingsCommand::execute(const vector<string>& tokens)
{
	try {
		switch (tokens.size()) {
		case 1:
			settingsConfig.saveSetting();
			break;
		case 2:
			settingsConfig.saveSetting(tokens[1]);
			break;
		default:
			throw SyntaxError();
		}
	} catch (FileException& e) {
		throw CommandException(e.getMessage());
	}
	return "";
}

string SaveSettingsCommand::help(const vector<string>& /*tokens*/) const
{
	return "Save the current settings.";
}

void SaveSettingsCommand::tabCompletion(vector<string>& tokens) const
{
	if (tokens.size() == 2) {
		SystemFileContext context;
		completeFileName(tokens, context);
	}
}


// class LoadSettingsCommand

LoadSettingsCommand::LoadSettingsCommand(
		CommandController& commandController,
		SettingsConfig& settingsConfig_)
	: Command(commandController, "load_settings")
	, settingsConfig(settingsConfig_)
{
}

string LoadSettingsCommand::execute(const vector<string>& tokens)
{
	if (tokens.size() != 2) {
		throw SyntaxError();
	}
	SystemFileContext context;
	settingsConfig.loadSetting(context, tokens[1]);
	return "";
}

string LoadSettingsCommand::help(const vector<string>& /*tokens*/) const
{
	return "Load settings from given file.";
}

void LoadSettingsCommand::tabCompletion(vector<string>& tokens) const
{
	if (tokens.size() == 2) {
		SystemFileContext context;
		completeFileName(tokens, context);
	}
}

} // namespace openmsx
