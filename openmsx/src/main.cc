// $Id: main.cc 12814 2012-08-13 20:22:45Z m9710797 $

/*
 *  openmsx - the MSX emulator that aims for perfection
 *
 */

#include "Reactor.hh"
#include "GlobalCommandController.hh"
#include "CommandLineParser.hh"
#include "CliServer.hh"
#include "Interpreter.hh"
#include "Display.hh"
#include "EventDistributor.hh"
#include "RenderSettings.hh"
#include "EnumSetting.hh"
#include "MSXException.hh"
#include "StringOp.hh"
#include "Thread.hh"
#include "HostCPU.hh"
#ifdef _WIN32
#include "win32-arggen.hh"
#endif
#include <memory>
#include <iostream>
#include <exception>
#include <cstdlib>
#include <SDL.h>

using std::auto_ptr;
using std::cerr;
using std::endl;
using std::string;

namespace openmsx {

static void initializeSDL()
{
	int flags = 0;
#ifndef NDEBUG
	flags |= SDL_INIT_NOPARACHUTE;
#endif
	if (SDL_Init(flags) < 0) {
		throw FatalError(StringOp::Builder() <<
			"Couldn't init SDL: " << SDL_GetError());
	}

// In SDL 1.2.9 and before SDL_putenv has different semantics and is not
// guaranteed to exist on all platforms.
#if SDL_VERSION_ATLEAST(1, 2, 10)
	// On Mac OS X, send key combos like Cmd+H and Cmd+M to Cocoa, so it can
	// perform the corresponding actions.
#if defined(__APPLE__)
	SDL_putenv(const_cast<char*>("SDL_ENABLEAPPEVENTS=1"));
#endif
#endif
}

static void unexpectedExceptionHandler()
{
	cerr << "Unexpected exception." << endl;
}

static int main(int argc, char **argv)
{
	std::set_unexpected(unexpectedExceptionHandler);

	int err = 0;
	try {
		HostCPU::init(); // as early as possible

		// Constructing Reactor already causes parts of SDL to be used
		// and initialized. If we want to set environment variables
		// before this, we have to do it here...
		//
		// This is to make sure we get no annoying behaviour from SDL
		// with regards to CAPS lock. This only works in SDL 1.2.14 or
		// later, but it can't hurt to always set it (if we can rely on
		// SDL_putenv, so on 1.2.10+).
		//
		// On Mac OS X, Cocoa does not report CAPS lock release events.
		// The input driver inside SDL works around that by sending a
		// pressed;released combo when CAPS status changes. However,
		// because there is no time inbetween this does not give the
		// MSX BIOS a chance to see the CAPS key in a pressed state.

#if SDL_VERSION_ATLEAST(1, 2, 10)
		SDL_putenv(const_cast<char*>("SDL_DISABLE_LOCK_KEYS="
#if defined(__APPLE__)
			"0"
#else
			"1"
#endif
			));
#endif

		Thread::setMainThread();
		Reactor reactor;
		reactor.getGlobalCommandController().getInterpreter().init(argv[0]);
#ifdef _WIN32
		ArgumentGenerator arggen;
		argv = arggen.GetArguments(argc);
#endif
		CommandLineParser parser(reactor);
		parser.parse(argc, argv);
		CommandLineParser::ParseStatus parseStatus = parser.getParseStatus();

		if (parseStatus != CommandLineParser::EXIT) {
			initializeSDL();
			if (!parser.isHiddenStartup()) {
				reactor.getDisplay().getRenderSettings().
					getRenderer().restoreDefault();
				// Switching renderer requires events, handle
				// these events before continuing with the rest
				// of initialization. This fixes a bug where
				// you have a '-script bla.tcl' command line
				// argument where bla.tcl contains a line like
				// 'ext gfx9000'.
				reactor.getEventDistributor().deliverEvents();
			}
			if (parseStatus != CommandLineParser::TEST) {
				CliServer cliServer(reactor.getCommandController(),
				                    reactor.getEventDistributor(),
				                    reactor.getGlobalCliComm());
				reactor.run(parser);
			}
		}
	} catch (FatalError& e) {
		cerr << "Fatal error: " << e.getMessage() << endl;
		err = 1;
	} catch (MSXException& e) {
		cerr << "Uncaught exception: " << e.getMessage() << endl;
		err = 1;
	} catch (std::exception& e) {
		cerr << "Uncaught std::exception: " << e.what() << endl;
		err = 1;
	} catch (...) {
		cerr << "Uncaught exception of unexpected type." << endl;
		err = 1;
	}
	// Clean up.
	if (SDL_WasInit(SDL_INIT_EVERYTHING)) {
		SDL_Quit();
	}

	return err;
}

} // namespace openmsx

// Enter the openMSX namespace.
int main(int argc, char **argv)
{
	exit(openmsx::main(argc, argv)); // need exit() iso return on win32/SDL
}
