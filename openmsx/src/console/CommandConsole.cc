// $Id: CommandConsole.cc 12700 2012-07-07 20:21:13Z m9710797 $

#include "CommandConsole.hh"
#include "CommandException.hh"
#include "GlobalCommandController.hh"
#include "Completer.hh"
#include "Interpreter.hh"
#include "Keys.hh"
#include "FileContext.hh"
#include "FileException.hh"
#include "FileOperations.hh"
#include "CliComm.hh"
#include "SettingsConfig.hh"
#include "XMLElement.hh"
#include "InputEvents.hh"
#include "Display.hh"
#include "EventDistributor.hh"
#include "GlobalSettings.hh"
#include "BooleanSetting.hh"
#include "IntegerSetting.hh"
#include "Version.hh"
#include "checked_cast.hh"
#include "utf8_unchecked.hh"
#include "StringOp.hh"
#include "ScopedAssign.hh"
#include <algorithm>
#include <fstream>
#include <cassert>

using std::min;
using std::max;
using std::string;
using std::make_pair;

namespace openmsx {

// class ConsoleLine

ConsoleLine::ConsoleLine()
{
}

ConsoleLine::ConsoleLine(string_ref line_, unsigned rgb)
	: line(line_.data(), line_.size())
	, chunks(1, make_pair(rgb, 0))
{
}

void ConsoleLine::addChunk(string_ref text, unsigned rgb)
{
	chunks.push_back(make_pair(rgb, string_ref::size_type(line.size())));
	line.append(text.data(), string_ref::size_type(text.size()));
}

unsigned ConsoleLine::numChars() const
{
	return utf8::unchecked::size(line);
}

const string& ConsoleLine::str() const
{
	return line;
}

unsigned ConsoleLine::numChunks() const
{
	return unsigned(chunks.size());
}

unsigned ConsoleLine::chunkColor(unsigned i) const
{
	assert(i < chunks.size());
	return chunks[i].first;
}

string_ref ConsoleLine::chunkText(unsigned i) const
{
	assert(i < chunks.size());
	string_ref::size_type pos = chunks[i].second;
	string_ref::size_type len = ((i + 1) == chunks.size())
	                          ? string_ref::npos
	                          : chunks[i + 1].second - pos;
	return string_ref(line).substr(pos, len);
}

ConsoleLine ConsoleLine::substr(unsigned pos, unsigned len) const
{
	ConsoleLine result;
	if (chunks.empty()) {
		assert(line.empty());
		assert(pos == 0);
		return result;
	}

	string::const_iterator begin = line.begin();
	utf8::unchecked::advance(begin, pos);
	string::const_iterator end = begin;
	while (len-- && (end != line.end())) {
		utf8::unchecked::next(end);
	}
	result.line.assign(begin, end);

	unsigned bpos = begin - line.begin();
	unsigned bend = end   - line.begin();
	unsigned i = 1;
	while ((i < chunks.size()) && (chunks[i].second <= bpos)) {
		++i;
	}
	result.chunks.push_back(make_pair(chunks[i - 1].first, 0));
	while ((i < chunks.size()) && (chunks[i].second < bend)) {
		result.chunks.push_back(make_pair(chunks[i].first,
		                                  chunks[i].second - bpos));
		++i;
	}
	return result;
}

// class CommandConsole

static const char* const PROMPT_NEW  = "> ";
static const char* const PROMPT_CONT = "| ";
static const char* const PROMPT_BUSY = "*busy*";

CommandConsole::CommandConsole(
		GlobalCommandController& commandController_,
		EventDistributor& eventDistributor_,
		Display& display_)
	: commandController(commandController_)
	, eventDistributor(eventDistributor_)
	, display(display_)
	, consoleSetting(new BooleanSetting(commandController, "console",
		"turns console display on/off", false, Setting::DONT_SAVE))
	, historySizeSetting(new IntegerSetting(commandController, "console_history_size",
		"amount of commands kept in console history", 100, 0, 10000))
	, removeDoublesSetting(new BooleanSetting(commandController,
		"console_remove_doubles", "don't add the command to history if "
		"it's the same as the previous one", true))
	, executingCommand(false)
{
	resetScrollBack();
	prompt = PROMPT_NEW;
	newLineConsole(prompt);
	putPrompt();
	loadHistory();
	Completer::setOutput(this);

	print(Version::FULL_VERSION);
	print(string(Version::FULL_VERSION.size(), '-'));
	print("\n"
	      "General information about openMSX is available at "
	      "http://www.openmsx.org.\n"
	      "\n"
	      "Type 'help' to see a list of available commands "
	      "(use <PgUp>/<PgDn> to scroll).\n"
	      "Or read the Console Command Reference in the manual.\n"
	      "\n");

	commandController.getInterpreter().setOutput(this);
	eventDistributor.registerEventListener(
		OPENMSX_KEY_DOWN_EVENT, *this, EventDistributor::CONSOLE);
	// also listen to KEY_UP events, so that we can consume them
	eventDistributor.registerEventListener(
		OPENMSX_KEY_UP_EVENT, *this, EventDistributor::CONSOLE);
}

CommandConsole::~CommandConsole()
{
	eventDistributor.unregisterEventListener(OPENMSX_KEY_DOWN_EVENT, *this);
	eventDistributor.unregisterEventListener(OPENMSX_KEY_UP_EVENT, *this);
	commandController.getInterpreter().setOutput(NULL);
	Completer::setOutput(NULL);
}

void CommandConsole::saveHistory()
{
	try {
		UserFileContext context("console");
		std::ofstream outputfile;
		FileOperations::openofstream(outputfile,
		        context.resolveCreate("history.txt"));
		if (!outputfile) {
			throw FileException(
				"Error while saving the console history.");
		}
		for (History::const_iterator it = history.begin();
		     it != history.end(); ++it) {
			outputfile << string_ref(*it).substr(string_ref::size_type(prompt.size())) << '\n';
		}
	} catch (FileException& e) {
		commandController.getCliComm().printWarning(e.getMessage());
	}
}

void CommandConsole::loadHistory()
{
	try {
		UserFileContext context("console");
		std::ifstream inputfile(
		        context.resolveCreate("history.txt").c_str());
		if (!inputfile) {
			throw FileException(
				"Error while loading the console history.");
		}
		string line;
		while (inputfile) {
			getline(inputfile, line);
			if (!line.empty()) {
				putCommandHistory(prompt + line);
			}
		}
	} catch (FileException& e) {
		PRT_DEBUG(e.getMessage());
		(void)&e; // Prevent warning
	}
}

BooleanSetting& CommandConsole::getConsoleSetting()
{
	return *consoleSetting.get();
}

void CommandConsole::getCursorPosition(unsigned& xPosition, unsigned& yPosition) const
{
	xPosition = cursorPosition % getColumns();
	unsigned num = lines[0].numChars() / getColumns();
	yPosition = num - (cursorPosition / getColumns());
}

unsigned CommandConsole::getScrollBack() const
{
	return consoleScrollBack;
}

ConsoleLine CommandConsole::getLine(unsigned line) const
{
	unsigned count = 0;
	for (unsigned buf = 0; buf < lines.size(); ++buf) {
		count += (lines[buf].numChars() / getColumns()) + 1;
		if (count > line) {
			return lines[buf].substr(
				(count - line - 1) * getColumns(),
				getColumns());
		}
	}
	return ConsoleLine();
}

int CommandConsole::signalEvent(const shared_ptr<const Event>& event)
{
	const KeyEvent& keyEvent = checked_cast<const KeyEvent&>(*event);
	if (!consoleSetting->getValue()) {
		return 0;
	}

	// If the console is open then don't pass the event to the MSX
	// (whetever the (keyboard) event is). If the event has a meaning for
	// the console, then also don't pass the event to the hotkey system.
	// For example PgUp, PgDown are keys that have both a meaning in the
	// console and are used by standard key bindings.
	if (event->getType() == OPENMSX_KEY_DOWN_EVENT) {
		if (!executingCommand) {
			if (handleEvent(keyEvent)) {
				// event was used
				display.repaintDelayed(40000); // 25fps
				return EventDistributor::HOTKEY |
				       EventDistributor::MSX;
			}
		} else {
			// For commands that take a long time to execute (e.g.
			// a loadstate that needs to create a filepool index),
			// we also send events during the execution (so that
			// we can show progress on the OSD). In that case
			// ignore extra input events.
		}
	} else {
		assert(event->getType() == OPENMSX_KEY_UP_EVENT);
	}
	return EventDistributor::MSX;
}

bool CommandConsole::handleEvent(const KeyEvent& keyEvent)
{
	Keys::KeyCode keyCode = keyEvent.getKeyCode();
	int key = keyCode &  Keys::K_MASK;
	int mod = keyCode & ~Keys::K_MASK;
	word chr = keyEvent.getUnicode();

	bool used = true;
	switch (mod) {
	case Keys::KM_CTRL:
		switch (key) {
		case Keys::K_H:
			backspace();
			break;
		case Keys::K_A:
			cursorPosition = unsigned(prompt.size());
			break;
		case Keys::K_E:
			cursorPosition = lines[0].numChars();
			break;
		case Keys::K_C:
			clearCommand();
			break;
		default:
			used = false;
			break;
		}
		break;
	case Keys::KM_SHIFT:
		switch (key) {
		case Keys::K_PAGEUP:
			scroll(max<int>(getRows() - 1, 1));
			break;
		case Keys::K_PAGEDOWN:
			scroll(-max<int>(getRows() - 1, 1));
			break;
		default:
			if (chr) {
				normalKey(chr);
			} else {
				used = false;
			}
			break;
		}
		break;
	case Keys::KM_MODE: // e.g. to type '1-9' on a N900
	case Keys::KM_ALT:  // e.g. to type | [ ] on a azerty keyboard layout
	// Don't add the META modifier here because that will break the Cmd+L
	// hotkey on MacOSX to toggle the console.
	case 0:
		switch (key) {
		case Keys::K_PAGEUP:
			scroll(1);
			break;
		case Keys::K_PAGEDOWN:
			scroll(-1);
			break;
		case Keys::K_UP:
			prevCommand();
			break;
		case Keys::K_DOWN:
			nextCommand();
			break;
		case Keys::K_BACKSPACE:
			backspace();
			break;
		case Keys::K_DELETE:
			delete_key();
			break;
		case Keys::K_TAB:
			tabCompletion();
			break;
		case Keys::K_RETURN:
		case Keys::K_KP_ENTER:
			commandExecute();
			cursorPosition = unsigned(prompt.size());
			break;
		case Keys::K_LEFT:
			if (cursorPosition > prompt.size()) {
				--cursorPosition;
			}
			break;
		case Keys::K_RIGHT:
			if (cursorPosition < lines[0].numChars()) {
				++cursorPosition;
			}
			break;
		case Keys::K_HOME:
			cursorPosition = unsigned(prompt.size());
			break;
		case Keys::K_END:
			cursorPosition = lines[0].numChars();
			break;
		default:
			if (chr) {
				normalKey(chr);
			} else {
				used = false;
			}
			break;
		}
		break;
	default:
		used = false;
	}
	return used;
}

void CommandConsole::setColumns(unsigned columns_)
{
	columns = columns_;
}

unsigned CommandConsole::getColumns() const
{
	return columns;
}

void CommandConsole::setRows(unsigned rows_)
{
	rows = rows_;
}

unsigned CommandConsole::getRows() const
{
	return rows;
}

void CommandConsole::output(string_ref text)
{
	print(text);
}

unsigned CommandConsole::getOutputColumns() const
{
	return getColumns();
}

void CommandConsole::print(string_ref text, unsigned rgb)
{
	while (true) {
		string_ref::size_type pos = text.find('\n');
		newLineConsole(ConsoleLine(text.substr(0, pos), rgb));
		if (pos == string_ref::npos) return;
		text = text.substr(pos + 1); // skip newline
		if (text.empty()) return;
	}
}

void CommandConsole::newLineConsole(string_ref line)
{
	newLineConsole(ConsoleLine(line));
}

void CommandConsole::newLineConsole(ConsoleLine line)
{
	if (lines.isFull()) {
		lines.removeBack();
	}
	ConsoleLine tmp = lines[0];
	lines[0] = line;
	lines.addFront(tmp);
}

void CommandConsole::putCommandHistory(const string& command)
{
	// TODO don't store PROMPT as part of history
	if (command == prompt) {
		return;
	}
	if (removeDoublesSetting->getValue() && !history.empty()
		&& (history.back() == command)) {
		return;
	}

	history.push_back(command);

	// if necessary, shrink history to the desired (smaller) size
	while (history.size() > unsigned(historySizeSetting->getValue())) {
		history.pop_front();
	}
}

void CommandConsole::commandExecute()
{
	resetScrollBack();
	putCommandHistory(lines[0].str());
	saveHistory(); // save at this point already, so that we don't lose history in case of a crash

	commandBuffer += lines[0].str().substr(prompt.size()) + '\n';
	newLineConsole(lines[0]);
	if (commandController.isComplete(commandBuffer)) {
		// Normally the busy promt is NOT shown (not even very briefly
		// because the screen is not redrawn), though for some commands
		// that potentially take a long time to execute, we explictly
		// send events, see also comment in handleEvent().
		prompt = PROMPT_BUSY;
		putPrompt();

		try {
			ScopedAssign<bool> sa(executingCommand, true);
			string result = commandController.executeCommand(
				commandBuffer);
			if (!result.empty()) {
				print(result);
			}
		} catch (CommandException& e) {
			print(e.getMessage(), 0xff0000);
		}
		commandBuffer.clear();
		prompt = PROMPT_NEW;
	} else {
		prompt = PROMPT_CONT;
	}
	putPrompt();
}

ConsoleLine CommandConsole::highLight(string_ref line)
{
	assert(line.starts_with(prompt));
	string_ref command = line.substr(string_ref::size_type(prompt.size()));
	ConsoleLine result;
	result.addChunk(prompt, 0xffffff);

	TclParser parser = commandController.getInterpreter().parse(command);
	string colors = parser.getColors();
	assert(colors.size() == command.size());

	unsigned pos = 0;
	while (pos != colors.size()) {
		char col = colors[pos];
		unsigned pos2 = pos++;
		while ((pos != colors.size()) && (colors[pos] == col)) {
			++pos;
		}
		// TODO make these color configurable?
		unsigned rgb;
		switch (col) {
		case 'E': rgb = 0xff0000; break; // error
		case 'c': rgb = 0x5c5cff; break; // comment
		case 'v': rgb = 0x00ffff; break; // variable
		case 'l': rgb = 0xff00ff; break; // literal
		case 'p': rgb = 0xcdcd00; break; // proc
		case 'o': rgb = 0x00cdcd; break; // operator
		default:  rgb = 0xffffff; break; // other
		}
		result.addChunk(command.substr(pos2, pos - pos2), rgb);
	}
	return result;
}

void CommandConsole::putPrompt()
{
	commandScrollBack = history.end();
	currentLine = prompt;
	lines[0] = highLight(currentLine);
	cursorPosition = unsigned(prompt.size());
}

void CommandConsole::tabCompletion()
{
	resetScrollBack();
	unsigned pl = unsigned(prompt.size());
	string_ref front = utf8::unchecked::substr(lines[0].str(), pl, cursorPosition - pl);
	string_ref back  = utf8::unchecked::substr(lines[0].str(), cursorPosition);
	string newFront = commandController.tabCompletion(front);
	cursorPosition = pl + utf8::unchecked::size(newFront);
	currentLine = prompt + newFront + back;
	lines[0] = highLight(currentLine);
}

void CommandConsole::scroll(int delta)
{
	consoleScrollBack = min<int>(max(consoleScrollBack + delta, 0),
	                             lines.size());
}

void CommandConsole::prevCommand()
{
	resetScrollBack();
	if (history.empty()) {
		return; // no elements
	}
	bool match = false;
	History::const_iterator tempScrollBack = commandScrollBack;
	while ((tempScrollBack != history.begin()) && !match) {
		--tempScrollBack;
		match = StringOp::startsWith(*tempScrollBack, currentLine);
	}
	if (match) {
		commandScrollBack = tempScrollBack;
		lines[0] = highLight(*commandScrollBack);
		cursorPosition = lines[0].numChars();
	}
}

void CommandConsole::nextCommand()
{
	resetScrollBack();
	if (commandScrollBack == history.end()) {
		return; // don't loop !
	}
	bool match = false;
	History::const_iterator tempScrollBack = commandScrollBack;
	while ((++tempScrollBack != history.end()) && !match) {
		match = StringOp::startsWith(*tempScrollBack, currentLine);
	}
	if (match) {
		--tempScrollBack; // one time to many
		commandScrollBack = tempScrollBack;
		lines[0] = highLight(*commandScrollBack);
	} else {
		commandScrollBack = history.end();
		lines[0] = highLight(currentLine);
	}
	cursorPosition = lines[0].numChars();
}

void CommandConsole::clearCommand()
{
	resetScrollBack();
	commandBuffer.clear();
	prompt = PROMPT_NEW;
	currentLine = prompt;
	lines[0] = highLight(currentLine);
	cursorPosition = unsigned(prompt.size());
}

void CommandConsole::backspace()
{
	resetScrollBack();
	if (cursorPosition > prompt.size()) {
		currentLine = lines[0].str();
		string::iterator begin = currentLine.begin();
		utf8::unchecked::advance(begin, cursorPosition - 1);
		string::iterator end = begin;
		utf8::unchecked::advance(end, 1);
		currentLine.erase(begin, end);
		lines[0] = highLight(currentLine);
		--cursorPosition;
	}
}

void CommandConsole::delete_key()
{
	resetScrollBack();
	if (lines[0].numChars() > cursorPosition) {
		currentLine = lines[0].str();
		string::iterator begin = currentLine.begin();
		utf8::unchecked::advance(begin, cursorPosition);
		string::iterator end = begin;
		utf8::unchecked::advance(end, 1);
		currentLine.erase(begin, end);
		lines[0] = highLight(currentLine);
	}
}

void CommandConsole::normalKey(word chr)
{
	assert(chr);
	resetScrollBack();
	currentLine = lines[0].str();
	string::iterator pos = currentLine.begin();
	utf8::unchecked::advance(pos, cursorPosition);
	utf8::unchecked::append(uint32_t(chr), inserter(currentLine, pos));
	lines[0] = highLight(currentLine);
	++cursorPosition;
}

void CommandConsole::resetScrollBack()
{
	consoleScrollBack = 0;
}

} // namespace openmsx
