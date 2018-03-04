// $Id: CliComm.hh 12625 2012-06-14 20:13:15Z m9710797 $

#ifndef CLICOMM_HH
#define CLICOMM_HH

#include "string_ref.hh"

namespace openmsx {

class CliComm
{
public:
	enum LogLevel {
		INFO,
		WARNING,
		LOGLEVEL_ERROR, // ERROR may give preprocessor name clashes
		PROGRESS,
		NUM_LEVELS // must be last
	};
	enum UpdateType {
		LED,
		SETTING,
		SETTINGINFO,
		HARDWARE,
		PLUG,
		UNPLUG,
		MEDIA,
		STATUS,
		EXTENSION,
		SOUNDDEVICE,
		CONNECTOR,
		NUM_UPDATES // must be last
	};

	virtual void log(LogLevel level, string_ref message) = 0;
	virtual void update(UpdateType type, string_ref name,
	                    string_ref value) = 0;

	// convenience methods (shortcuts for log())
	void printInfo    (string_ref message);
	void printWarning (string_ref message);
	void printError   (string_ref message);
	void printProgress(string_ref message);

	// string representations of the LogLevel and UpdateType enums
	static const char* const* getLevelStrings()  { return levelStr;  }
	static const char* const* getUpdateStrings() { return updateStr; }

protected:
	CliComm();
	virtual ~CliComm();

private:
	static const char* const levelStr [NUM_LEVELS];
	static const char* const updateStr[NUM_UPDATES];
};

} // namespace openmsx

#endif
