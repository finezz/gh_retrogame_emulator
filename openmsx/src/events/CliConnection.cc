// $Id: CliConnection.cc 12633 2012-06-14 20:41:30Z m9710797 $

// TODO:
// - To avoid any possible conflicts, anything called from "run" should be
//   locked.
// - Maybe document for each method whether it is called from the listener
//   thread or from the main thread?
// - Unsubscribe at CliComm after stream is closed.

#include "CliConnection.hh"
#include "EventDistributor.hh"
#include "Event.hh"
#include "CommandController.hh"
#include "CommandException.hh"
#include "XMLElement.hh"
#include "checked_cast.hh"
#include "cstdiop.hh"
#include "unistdp.hh"
#include "openmsx.hh"
#include "StringOp.hh"
#include <cassert>
#include <iostream>

#ifdef _WIN32
#include "SocketStreamWrapper.hh"
#include "SspiNegotiateServer.hh"
#endif

using std::string;

namespace openmsx {

// class CliCommandEvent

class CliCommandEvent : public Event
{
public:
	CliCommandEvent(const string& command_, const CliConnection* id_)
		: Event(OPENMSX_CLICOMMAND_EVENT)
		, command(command_), id(id_)
	{
	}
	const string& getCommand() const
	{
		return command;
	}
	const CliConnection* getId() const
	{
		return id;
	}
private:
	const string command;
	const CliConnection* id;
};


// class CliConnection

CliConnection::CliConnection(CommandController& commandController_,
                             EventDistributor& eventDistributor_)
	: thread(this)
	, commandController(commandController_)
	, eventDistributor(eventDistributor_)
{
	user_data.state = START;
	user_data.unknownLevel = 0;
	user_data.object = this;
	memset(&sax_handler, 0, sizeof(sax_handler));
	sax_handler.startElementNs = cb_start_element;
	sax_handler.endElementNs   = cb_end_element;
	sax_handler.characters     = cb_text;
	sax_handler.initialized    = XML_SAX2_MAGIC;

	parser_context = xmlCreatePushParserCtxt(&sax_handler, &user_data, 0, 0, 0);

	for (int i = 0; i < CliComm::NUM_UPDATES; ++i) {
		updateEnabled[i] = false;
	}

	eventDistributor.registerEventListener(OPENMSX_CLICOMMAND_EVENT, *this);
}

CliConnection::~CliConnection()
{
	eventDistributor.unregisterEventListener(OPENMSX_CLICOMMAND_EVENT, *this);
	xmlFreeParserCtxt(parser_context);
}

void CliConnection::setUpdateEnable(CliComm::UpdateType type, bool value)
{
	updateEnabled[type] = value;
}

bool CliConnection::getUpdateEnable(CliComm::UpdateType type) const
{
	return updateEnabled[type];
}

void CliConnection::log(CliComm::LogLevel level, string_ref message)
{
	const char* const* levelStr = CliComm::getLevelStrings();
	output(StringOp::Builder() <<
		"<log level=\"" << levelStr[level] << "\">" <<
		XMLElement::XMLEscape(message.str()) << "</log>\n");
}

void CliConnection::update(CliComm::UpdateType type, string_ref machine,
                              string_ref name, string_ref value)
{
	if (!getUpdateEnable(type)) return;

	const char* const* updateStr = CliComm::getUpdateStrings();
	StringOp::Builder tmp;
	tmp << "<update type=\"" << updateStr[type] << '\"';
	if (!machine.empty()) {
		tmp << " machine=\"" << machine << '\"';
	}
	if (!name.empty()) {
		tmp << " name=\"" << XMLElement::XMLEscape(name.str()) << '\"';
	}
	tmp << '>' << XMLElement::XMLEscape(value.str()) << "</update>\n";

	output(tmp);
}

void CliConnection::startOutput()
{
	output("<openmsx-output>\n");
}

void CliConnection::start()
{
	thread.start();
}

void CliConnection::end()
{
	output("</openmsx-output>\n");
	close();
}

void CliConnection::execute(const string& command)
{
	PRT_DEBUG("CliConnection::execute: " << command);
	eventDistributor.distributeEvent(new CliCommandEvent(command, this));
}

static string reply(const string& message, bool status)
{
	return StringOp::Builder() <<
		"<reply result=\"" << (status ? "ok" : "nok") << "\">" <<
		XMLElement::XMLEscape(message) << "</reply>\n";
}

int CliConnection::signalEvent(const shared_ptr<const Event>& event)
{
	const CliCommandEvent& commandEvent =
		checked_cast<const CliCommandEvent&>(*event);
	if (commandEvent.getId() == this) {
		try {
			string result = commandController.executeCommand(
				commandEvent.getCommand(), this);
			PRT_DEBUG("CliConnection::signalEvent result: " << result);
			output(reply(result, true));
		} catch (CommandException& e) {
			string result = e.getMessage() + '\n';
			output(reply(result, false));
		}
	}
	return 0;
}

void CliConnection::cb_start_element(
	void* user_data,
	const xmlChar* localname, const xmlChar* /*prefix*/, const xmlChar* /*uri*/,
	int /*nb_namespaces*/, const xmlChar** /*namespaces*/,
	int /*nb_attributes*/, int /*nb_defaulted*/, const xmlChar** /*attrs*/
	)
{
	ParseState* parseState = static_cast<ParseState*>(user_data);
	if (parseState->unknownLevel) {
		++(parseState->unknownLevel);
		return;
	}
	switch (parseState->state) {
		case START:
			if (strcmp(reinterpret_cast<const char*>(localname),
					"openmsx-control") == 0) {
				parseState->state = TAG_OPENMSX;
			} else {
				++(parseState->unknownLevel);
			}
			break;
		case TAG_OPENMSX:
			if (strcmp(reinterpret_cast<const char*>(localname),
					"command") == 0) {
				parseState->state = TAG_COMMAND;
			} else {
				++(parseState->unknownLevel);
			}
			break;
		default:
			++(parseState->unknownLevel);
			break;
	}
	parseState->content.clear();
}

void CliConnection::cb_end_element(
	void* user_data,
	const xmlChar* /*localname*/, const xmlChar* /*prefix*/,
	const xmlChar* /*uri*/
	)
{
	ParseState* parseState = static_cast<ParseState*>(user_data);
	if (parseState->unknownLevel) {
		--(parseState->unknownLevel);
		return;
	}
	switch (parseState->state) {
		case TAG_OPENMSX:
			parseState->object->end();
			parseState->state = END;
			break;
		case TAG_COMMAND:
			parseState->object->execute(parseState->content);
			parseState->state = TAG_OPENMSX;
			break;
		default:
			break;
	}
}

void CliConnection::cb_text(void* user_data, const xmlChar* chars, int len)
{
	ParseState* parseState = static_cast<ParseState*>(user_data);
	if (parseState->state == TAG_COMMAND) {
		parseState->content.append(reinterpret_cast<const char*>(chars), len);
	}
}


// class StdioConnection

static const int BUF_SIZE = 4096;
StdioConnection::StdioConnection(CommandController& commandController,
                                 EventDistributor& eventDistributor)
	: CliConnection(commandController, eventDistributor)
	, ok(true)
{
	startOutput();
	start();
}

StdioConnection::~StdioConnection()
{
	end();
}

void StdioConnection::run()
{
	// runs in helper thread
	while (ok) {
		char buf[BUF_SIZE];
		int n = read(STDIN_FILENO, buf, sizeof(buf));
		if (n > 0) {
			xmlParseChunk(parser_context, buf, n, 0);
		} else if (n < 0) {
			close();
			break;
		}
	}
}

void StdioConnection::output(string_ref message)
{
	if (ok) {
		std::cout << message << std::flush;
	}
}

void StdioConnection::close()
{
	// don't close stdin/out/err
	ok = false;
}


#ifdef _WIN32
// class PipeConnection

// INVALID_HANDLE_VALUE is #defined as  (HANDLE)(-1)
// but that gives a old-style-cast warning
static const HANDLE OPENMSX_INVALID_HANDLE_VALUE = reinterpret_cast<HANDLE>(-1);

PipeConnection::PipeConnection(CommandController& commandController,
                               EventDistributor& eventDistributor,
                               string_ref name)
	: CliConnection(commandController, eventDistributor)
{
	string pipeName = "\\\\.\\pipe\\" + name;
	pipeHandle = CreateFileA(pipeName.c_str(), GENERIC_READ, 0, NULL,
	                         OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (pipeHandle == OPENMSX_INVALID_HANDLE_VALUE) {
		char msg[256];
		snprintf(msg, 255, "Error reopening pipefile '%s': error %u",
		         pipeName.c_str(), unsigned(GetLastError()));
		throw FatalError(msg);
	}

	shutdownEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
	if (shutdownEvent == NULL) {
		throw FatalError(StringOp::Builder() <<
			"Error creating shutdown event: " << GetLastError());
	}

	startOutput();
	start();
}

PipeConnection::~PipeConnection()
{
	end();

	CloseHandle(shutdownEvent);
}

void InitOverlapped(LPOVERLAPPED overlapped)
{
	ZeroMemory(overlapped, sizeof(*overlapped));
	overlapped->hEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
	if (overlapped->hEvent == NULL) {
		throw FatalError(StringOp::Builder() <<
			"Error creating overlapped event: " << GetLastError());
	}
}

void ClearOverlapped(LPOVERLAPPED overlapped)
{
	if (overlapped->hEvent) {
		CloseHandle(overlapped->hEvent);
		overlapped->hEvent = NULL;
	}
}

void PipeConnection::run()
{
	// runs in helper thread
	OVERLAPPED overlapped;
	InitOverlapped(&overlapped);
	HANDLE waitHandles[2] = { shutdownEvent, overlapped.hEvent };

	while (pipeHandle != OPENMSX_INVALID_HANDLE_VALUE) {
		char buf[BUF_SIZE];
		if (!ReadFile(pipeHandle, buf, BUF_SIZE, NULL, &overlapped) &&
			GetLastError() != ERROR_IO_PENDING) {
			break; // Pipe broke
		}
		DWORD wait = WaitForMultipleObjects(2, waitHandles, FALSE, INFINITE);
		if (wait == WAIT_OBJECT_0 + 1) {
			DWORD bytesRead;
			if (!GetOverlappedResult(pipeHandle, &overlapped, &bytesRead, TRUE)) {
				break; // Pipe broke
			}
			xmlParseChunk(parser_context, buf, bytesRead, 0);
		}
		else if (wait == WAIT_OBJECT_0) {
			break; // Shutdown
		}
		else {
			throw FatalError(StringOp::Builder() <<
				"WaitForMultipleObjects returned unexpectedly: " << wait);
		}
	}

	ClearOverlapped(&overlapped);

	// We own the pipe handle, so close it here
	CloseHandle(pipeHandle);
	pipeHandle = OPENMSX_INVALID_HANDLE_VALUE;
}

void PipeConnection::output(string_ref message)
{
	if (pipeHandle != OPENMSX_INVALID_HANDLE_VALUE) {
		std::cout << message << std::flush;
	}
}

void PipeConnection::close()
{
	SetEvent(shutdownEvent);
	thread.join();
	assert(pipeHandle == OPENMSX_INVALID_HANDLE_VALUE);
}
#endif // _WIN32


// class SocketConnection

SocketConnection::SocketConnection(CommandController& commandController,
                                   EventDistributor& eventDistributor,
                                   SOCKET sd_)
	: CliConnection(commandController, eventDistributor)
	, sem(1), sd(sd_), established(false)
{
	start();
}

SocketConnection::~SocketConnection()
{
	end();
}

void SocketConnection::run()
{
	// runs in helper thread
#ifdef _WIN32
	bool ok;
	{
		ScopedLock lock(sem);
		// Authenticate and authorize the caller
		SocketStreamWrapper stream(sd);
		SspiNegotiateServer server(stream);
		ok = server.Authenticate() && server.Authorize();
	}
	if (!ok) {
		close();
		return;
	}
#endif
	// Start output element
	established = true; // TODO needs locking?
	startOutput();

	// TODO is locking correct?
	// No need to lock in this thread because we don't write to 'sd'
	// and 'sd' only gets written to in this thread.
	while (true) {
		if (sd == OPENMSX_INVALID_SOCKET) return;
		char buf[BUF_SIZE];
		int n = sock_recv(sd, buf, BUF_SIZE);
		if (n > 0) {
			xmlParseChunk(parser_context, buf, n, 0);
		} else if (n < 0) {
			close();
			break;
		}
	}
}

void SocketConnection::output(string_ref message)
{
	if (!established) { // TODO needs locking?
		// Connection isn't authorized yet (and opening tag is not
		// yet send). Ignore log and update messages for now.
		return;
	}
	const char* data = message.data();
	unsigned pos = 0;
	size_t bytesLeft = message.size();
	while (bytesLeft) {
		int bytesSend;
		{
			ScopedLock lock(sem);
			if (sd == OPENMSX_INVALID_SOCKET) return;
			bytesSend = sock_send(sd, &data[pos], bytesLeft);
		}
		if (bytesSend > 0) {
			bytesLeft -= bytesSend;
			pos += bytesSend;
		} else {
			close();
			break;
		}
	}
}

void SocketConnection::close()
{
	ScopedLock lock(sem);
	if (sd != OPENMSX_INVALID_SOCKET) {
		SOCKET _sd = sd;
		sd = OPENMSX_INVALID_SOCKET;
		sock_close(_sd);
	}
}

} // namespace openmsx
