// $Id: PrinterPortLogger.hh 12631 2012-06-14 20:18:24Z m9710797 $

#ifndef MSXPRINTERPORTLOGGER_HH
#define MSXPRINTERPORTLOGGER_HH

#include "PrinterPortDevice.hh"
#include "serialize_meta.hh"
#include <memory>

namespace openmsx {

class CommandController;
class File;
class FilenameSetting;

class PrinterPortLogger : public PrinterPortDevice
{
public:
	explicit PrinterPortLogger(CommandController& commandController);
	virtual ~PrinterPortLogger();

	// PrinterPortDevice
	virtual bool getStatus(EmuTime::param time);
	virtual void setStrobe(bool strobe, EmuTime::param time);
	virtual void writeData(byte data, EmuTime::param time);

	// Pluggable
	virtual const std::string& getName() const;
	virtual string_ref getDescription() const;
	virtual void plugHelper(Connector& connector, EmuTime::param time);
	virtual void unplugHelper(EmuTime::param time);

	template<typename Archive>
	void serialize(Archive& ar, unsigned version);

private:
	const std::auto_ptr<FilenameSetting> logFilenameSetting;
	std::auto_ptr<File> file;
	byte toPrint;
	bool prevStrobe;
};

} // namespace openmsx

#endif
