// $Id: V9990.cc 12695 2012-07-06 09:21:11Z m9710797 $

#include "V9990.hh"
#include "Display.hh"
#include "RendererFactory.hh"
#include "V9990VRAM.hh"
#include "V9990CmdEngine.hh"
#include "V9990Renderer.hh"
#include "SimpleDebuggable.hh"
#include "Reactor.hh"
#include "serialize.hh"
#include "unreachable.hh"
#include <cassert>
#include <cstring>

namespace openmsx {

class V9990RegDebug : public SimpleDebuggable
{
public:
	explicit V9990RegDebug(V9990& v9990);
	virtual byte read(unsigned address);
	virtual void write(unsigned address, byte value, EmuTime::param time);
private:
	V9990& v9990;
};

class V9990PalDebug : public SimpleDebuggable
{
public:
	explicit V9990PalDebug(V9990& v9990);
	virtual byte read(unsigned address);
	virtual void write(unsigned address, byte value, EmuTime::param time);
private:
	V9990& v9990;
};


static const byte ALLOW_READ  = 1;
static const byte ALLOW_WRITE = 2;
static const byte NO_ACCESS = 0;
static const byte RD_ONLY   = ALLOW_READ;
static const byte WR_ONLY   = ALLOW_WRITE;
static const byte RD_WR     = ALLOW_READ | ALLOW_WRITE;
static const byte regAccess[64] = {
	WR_ONLY, WR_ONLY, WR_ONLY,          // VRAM Write Address
	WR_ONLY, WR_ONLY, WR_ONLY,          // VRAM Read Address
	RD_WR, RD_WR,                       // Screen Mode
	RD_WR,                              // Control
	RD_WR, RD_WR, RD_WR, RD_WR,         // Interrupt
	WR_ONLY,                            // Palette Control
	WR_ONLY,                            // Palette Pointer
	RD_WR,                              // Back Drop Color
	RD_WR,                              // Display Adjust
	RD_WR, RD_WR, RD_WR, RD_WR,         // Scroll Control A
	RD_WR, RD_WR, RD_WR, RD_WR,         // Scroll Control B
	RD_WR,                              // Sprite Pattern Table Adress
	RD_WR,                              // LCD Control
	RD_WR,                              // Priority Control
	WR_ONLY,                            // Sprite Palette Control
	NO_ACCESS, NO_ACCESS, NO_ACCESS,    // 3x not used
	WR_ONLY, WR_ONLY, WR_ONLY, WR_ONLY, // Cmd Parameter Src XY
	WR_ONLY, WR_ONLY, WR_ONLY, WR_ONLY, // Cmd Parameter Dest XY
	WR_ONLY, WR_ONLY, WR_ONLY, WR_ONLY, // Cmd Parameter Size XY
	WR_ONLY, WR_ONLY, WR_ONLY, WR_ONLY, // Cmd Parameter Arg, LogOp, WrtMask
	WR_ONLY, WR_ONLY, WR_ONLY, WR_ONLY, // Cmd Parameter Font Color
	WR_ONLY, RD_ONLY, RD_ONLY,          // Cmd Parameter OpCode, Border X
	NO_ACCESS, NO_ACCESS, NO_ACCESS,    // registers 55-63
	NO_ACCESS, NO_ACCESS, NO_ACCESS,
	NO_ACCESS, NO_ACCESS, NO_ACCESS
};

// -------------------------------------------------------------------------
// Constructor & Destructor
// -------------------------------------------------------------------------

V9990::V9990(const DeviceConfig& config)
	: MSXDevice(config)
	, Schedulable(MSXDevice::getScheduler())
	, v9990RegDebug(new V9990RegDebug(*this))
	, v9990PalDebug(new V9990PalDebug(*this))
	, irq(getMotherBoard(), getName() + ".IRQ")
	, display(getReactor().getDisplay())
	, frameStartTime(Schedulable::getCurrentTime())
	, hScanSyncTime(Schedulable::getCurrentTime())
	, pendingIRQs(0)
	, externalVideoSource(false)
{
	// clear regs TODO find realistic init values
	memset(regs, 0, sizeof(regs));
	calcDisplayMode();

	// initialize palette
	for (int i = 0; i < 64; ++i) {
		palette[4 * i + 0] = 0x9F;
		palette[4 * i + 1] = 0x1F;
		palette[4 * i + 2] = 0x1F;
		palette[4 * i + 3] = 0x00;
	}

	// create VRAM
	EmuTime::param time = Schedulable::getCurrentTime();
	vram.reset(new V9990VRAM(*this, time));

	// create Command Engine
	cmdEngine.reset(new V9990CmdEngine(
		*this, time, display.getRenderSettings()));
	vram->setCmdEngine(*cmdEngine);

	// Start with NTSC timing
	palTiming = false;
	interlaced = false;
	setVerticalTiming();

	// Initialise rendering system
	isDisplayArea = false;
	displayEnabled = false; // avoid UMR (used by createRenderer())
	createRenderer(time);

	powerUp(time);
	display.attach(*this);
}

V9990::~V9990()
{
	display.detach(*this);
}

// -------------------------------------------------------------------------
// MSXDevice
// -------------------------------------------------------------------------

void V9990::powerUp(EmuTime::param time)
{
	vram->clear();
	reset(time);
}

void V9990::reset(EmuTime::param time)
{
	removeSyncPoint(V9990_VSYNC);
	removeSyncPoint(V9990_DISPLAY_START);
	removeSyncPoint(V9990_VSCAN);
	removeSyncPoint(V9990_HSCAN);
	removeSyncPoint(V9990_SET_MODE);

	// Clear registers / ports
	memset(regs, 0, sizeof(regs));
	status = 0;
	regSelect = 0xFF; // TODO check value for power-on and reset
	systemReset = false; // verified on real MSX
	calcDisplayMode();

	isDisplayArea = false;
	displayEnabled = false;
	superimposing = false;

	// Reset IRQs
	writeIO(INTERRUPT_FLAG, 0xFF, time);

	palTiming = false;
	// Reset sub-systems
	cmdEngine->sync(time);
	renderer->reset(time);
	cmdEngine->reset(time);

	// Init scheduling
	frameStart(time);
}

byte V9990::readIO(word port, EmuTime::param time)
{
	port &= 0x0F;

	// calculate return value (mostly uses peekIO)
	byte result;
	switch (port) {
	case COMMAND_DATA:
		result = cmdEngine->getCmdData(time);
		break;

	case VRAM_DATA:
	case PALETTE_DATA:
	case REGISTER_DATA:
	case INTERRUPT_FLAG:
	case STATUS:
	case KANJI_ROM_0:
	case KANJI_ROM_1:
	case KANJI_ROM_2:
	case KANJI_ROM_3:
	case REGISTER_SELECT:
	case SYSTEM_CONTROL:
	default:
		result = peekIO(port, time);
	}
	// TODO verify this, especially REGISTER_DATA
	if (systemReset) return result; // no side-effects

	// execute side-effects
	switch (port) {
	case VRAM_DATA:
		if (!(regs[VRAM_READ_ADDRESS_2] & 0x80)) {
			unsigned addr = getVRAMAddr(VRAM_READ_ADDRESS_0);
			setVRAMAddr(VRAM_READ_ADDRESS_0, addr + 1);
		}
		break;

	case PALETTE_DATA:
		if (!(regs[PALETTE_CONTROL] & 0x10)) {
			byte& palPtr = regs[PALETTE_POINTER];
			switch (palPtr & 3) {
			case 0:  palPtr += 1; break; // red
			case 1:  palPtr += 1; break; // green
			case 2:  palPtr += 2; break; // blue
			default: palPtr -= 3; break; // checked on real V9990
			}
		}
		break;

	case REGISTER_DATA:
		if (!(regSelect & 0x40)) {
			//regSelect = ( regSelect      & 0xC0) |
			//            ((regSelect + 1) & 0x3F);
			regSelect = (regSelect + 1) & ~0x40;
		}
		break;
	}
	return result;
}

byte V9990::peekIO(word port, EmuTime::param time) const
{
	byte result;
	switch (port & 0x0F) {
	case VRAM_DATA: {
		// TODO in 'systemReset' mode, this seems to hang the MSX
		unsigned addr = getVRAMAddr(VRAM_READ_ADDRESS_0);
		result = vram->readVRAMCPU(addr, time);
		break;
	}
	case PALETTE_DATA:
		result = palette[regs[PALETTE_POINTER]];
		break;

	case COMMAND_DATA:
		result = cmdEngine->peekCmdData(time);
		break;

	case REGISTER_DATA:
		result = readRegister(regSelect & 0x3F, time);
		break;

	case INTERRUPT_FLAG:
		result = pendingIRQs;
		break;

	case STATUS: {
		unsigned left   = getLeftBorder();
		unsigned right  = getRightBorder();
		unsigned top    = getTopBorder();
		unsigned bottom = getBottomBorder();
		unsigned ticks = getUCTicksThisFrame(time);
		unsigned x = ticks % V9990DisplayTiming::UC_TICKS_PER_LINE;
		unsigned y = ticks / V9990DisplayTiming::UC_TICKS_PER_LINE;
		bool hr = (x < left) || (right  <= x);
		bool vr = (y < top)  || (bottom <= y);

		result = cmdEngine->getStatus(time) |
		         (vr ? 0x40 : 0x00) |
		         (hr ? 0x20 : 0x00) |
		         (status & 0x06);
		break;
	}
	case KANJI_ROM_1:
	case KANJI_ROM_3:
		// not used in Gfx9000
		result = 0xFF; // TODO check
		break;

	case REGISTER_SELECT:
	case SYSTEM_CONTROL:
	case KANJI_ROM_0:
	case KANJI_ROM_2:
	default:
		// write-only
		result = 0xFF;
		break;
	}
	return result;
}

void V9990::writeIO(word port, byte val, EmuTime::param time)
{
	port &= 0x0F;
	switch (port) {
		case VRAM_DATA: {
			// write VRAM
			if (systemReset) {
				// TODO writes in systemReset mode seem to have
				//  'some' effect but it's not immediately clear
				//  what the exact behaviour is
				return;
			}
			unsigned addr = getVRAMAddr(VRAM_WRITE_ADDRESS_0);
			vram->writeVRAMCPU(addr, val, time);
			if (!(regs[VRAM_WRITE_ADDRESS_2] & 0x80)) {
				setVRAMAddr(VRAM_WRITE_ADDRESS_0, addr + 1);
			}
			break;
		}
		case PALETTE_DATA: {
			if (systemReset) {
				// Equivalent to writing 0 and keeping palPtr = 0
				// The above interpretation makes it similar to
				// writes to REGISTER_DATA, REGISTER_SELECT.
				writePaletteRegister(0, 0, time);
				return;
			}
			byte& palPtr = regs[PALETTE_POINTER];
			writePaletteRegister(palPtr, val, time);
			switch (palPtr & 3) {
				case 0:  palPtr += 1; break; // red
				case 1:  palPtr += 1; break; // green
				case 2:  palPtr += 2; break; // blue
				default: palPtr -= 3; break; // checked on real V9990
			}
			break;
		}
		case COMMAND_DATA:
			// systemReset state doesn't matter:
			//   command below has no effect in systemReset mode
			//assert(cmdEngine != NULL);
			cmdEngine->setCmdData(val, time);
			break;

		case REGISTER_DATA: {
			// write register
			if (systemReset) {
				// In systemReset mode, write has no effect,
				// but 'regSelect' is increased.
				// I don't know if write is ignored or a write
				// with val=0 is executed. Though both have the
				// same effect and the latter is more in line
				// with writes to PALETTE_DATA and
				// REGISTER_SELECT.
				val = 0;
			}
			writeRegister(regSelect & 0x3F, val, time);
			if (!(regSelect & 0x80)) {
				regSelect = ( regSelect      & 0xC0) |
				            ((regSelect + 1) & 0x3F);
			}
			break;
		}
		case REGISTER_SELECT:
			if (systemReset) {
				// Tested on real MSX. Also when no write is done
				// to this port, regSelect is not RESET when
				// entering/leaving systemReset mode.
				// This behavior is similar to PALETTE_DATA and
				// REGISTER_DATA.
				val = 0;
			}
			regSelect = val;
			break;

		case STATUS:
			// read-only, ignore writes
			break;

		case INTERRUPT_FLAG:
			// systemReset state doesn't matter:
			//   stuff below has no effect in systemReset mode
			pendingIRQs &= ~val;
			if (!(pendingIRQs & regs[INTERRUPT_0])) {
				irq.reset();
			}
			scheduleHscan(time);
			break;

		case SYSTEM_CONTROL: {
			// TODO investigate: does switching overscan mode
			//      happen at next line or next frame
			status = (status & 0xFB) | ((val & 1) << 2);
			syncAtNextLine(V9990_SET_MODE, time);

			bool newSystemReset = (val & 2) != 0;
			if (newSystemReset != systemReset) {
				systemReset = newSystemReset;
				if (systemReset) {
					// Enter systemReset mode
					//   Verified on real MSX: palette data
					//   and VRAM content are NOT reset.
					for (int i = 0; i < 64; ++i) {
						writeRegister(i, 0, time);
					}
					// TODO verify IRQ behaviour
					writeIO(INTERRUPT_FLAG, 0xFF, time);
				}
			}
			break;
		}
		case KANJI_ROM_0:
		case KANJI_ROM_1:
		case KANJI_ROM_2:
		case KANJI_ROM_3:
			// not used in Gfx9000, ignore
			break;

		default:
			// ignore
			break;
	}
}

// =========================================================================
// Private stuff
// =========================================================================

// -------------------------------------------------------------------------
// Schedulable
// -------------------------------------------------------------------------

void V9990::executeUntil(EmuTime::param time, int userData)
{
	switch (userData)  {
	case V9990_VSYNC:
		// Transition from one frame to the next
		renderer->frameEnd(time);
		frameStart(time);
		break;

	case V9990_DISPLAY_START:
		if (displayEnabled) {
			renderer->updateDisplayEnabled(true, time);
		}
		isDisplayArea = true;
		break;

	case V9990_VSCAN:
		if (isDisplayEnabled()) {
			renderer->updateDisplayEnabled(false, time);
		}
		isDisplayArea = false;
		raiseIRQ(VER_IRQ);
		break;

	case V9990_HSCAN:
		raiseIRQ(HOR_IRQ);
		break;

	case V9990_SET_MODE:
		calcDisplayMode();
		renderer->setDisplayMode(getDisplayMode(), time);
		renderer->setColorMode(getColorMode(), time);
		break;

	default:
		UNREACHABLE;
	}
}

// -------------------------------------------------------------------------
// VideoSystemChangeListener
// -------------------------------------------------------------------------

void V9990::preVideoSystemChange()
{
	renderer.reset();
}

void V9990::postVideoSystemChange()
{
	EmuTime::param time = Schedulable::getCurrentTime();
	createRenderer(time);
	renderer->frameStart(time);
}

// -------------------------------------------------------------------------
// V9990RegDebug
// -------------------------------------------------------------------------

V9990RegDebug::V9990RegDebug(V9990& v9990_)
	: SimpleDebuggable(v9990_.getMotherBoard(),
	                   v9990_.getName() + " regs", "V9990 registers", 0x40)
	, v9990(v9990_)
{
}

byte V9990RegDebug::read(unsigned address)
{
	return v9990.regs[address];
}

void V9990RegDebug::write(unsigned address, byte value, EmuTime::param time)
{
	v9990.writeRegister(address, value, time);
}

// -------------------------------------------------------------------------
// V9990PalDebug
// -------------------------------------------------------------------------

V9990PalDebug::V9990PalDebug(V9990& v9990_)
	: SimpleDebuggable(v9990_.getMotherBoard(),
	                   v9990_.getName() + " palette",
	                   "V9990 palette (format is R, G, B, 0).", 0x100)
	, v9990(v9990_)
{
}

byte V9990PalDebug::read(unsigned address)
{
	return v9990.palette[address];
}

void V9990PalDebug::write(unsigned address, byte value, EmuTime::param time)
{
	v9990.writePaletteRegister(address, value, time);
}

// -------------------------------------------------------------------------
// Private methods
// -------------------------------------------------------------------------

inline unsigned V9990::getVRAMAddr(RegisterId base) const
{
	return   regs[base + 0] +
	        (regs[base + 1] << 8) +
	       ((regs[base + 2] & 0x07) << 16);
}

inline void V9990::setVRAMAddr(RegisterId base, unsigned addr)
{
	regs[base + 0] =   addr &     0xFF;
	regs[base + 1] =  (addr &   0xFF00) >> 8;
	regs[base + 2] = ((addr & 0x070000) >> 16) | (regs[base + 2] & 0x80);
	// TODO check
}

byte V9990::readRegister(byte reg, EmuTime::param time) const
{
	// TODO sync(time) (if needed at all)
	if (systemReset) return 255; // verified on real MSX

	assert(reg < 64);
	byte result;
	if (regAccess[reg] & ALLOW_READ) {
		if (reg < CMD_PARAM_BORDER_X_0) {
			result = regs[reg];
		} else {
			word borderX = cmdEngine->getBorderX(time);
			result = (reg == CMD_PARAM_BORDER_X_0)
			       ? (borderX & 0xFF) : (borderX >> 8);
		}
	} else {
		result = 0xFF;
	}
	return result;
}

void V9990::syncAtNextLine(V9990SyncType type, EmuTime::param time)
{
	int line = getUCTicksThisFrame(time) / V9990DisplayTiming::UC_TICKS_PER_LINE;
	int ticks = (line + 1) * V9990DisplayTiming::UC_TICKS_PER_LINE;
	EmuTime nextTime = frameStartTime + ticks;
	setSyncPoint(nextTime, type);
}

void V9990::writeRegister(byte reg, byte val, EmuTime::param time)
{
	// Found this table by writing 0xFF to a register and reading
	// back the value (only works for read/write registers)
	static const byte regWriteMask[32] = {
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0x87, 0xFF, 0x83, 0x0F, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xDF, 0x07, 0xFF, 0xFF, 0xC1, 0x07,
		0x3F, 0xCF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
	};

	assert(reg < 64);
	if (!(regAccess[reg] & ALLOW_WRITE)) {
		// register not writable
		return;
	}
	if (reg >= CMD_PARAM_SRC_ADDRESS_0) {
		cmdEngine->setCmdReg(reg, val, time);
		return;
	}

	val &= regWriteMask[reg];

	// This optimization is not valid for the vertical scroll registers
	// TODO is this optimization still useful for other registers?
	//if (!change) return;

	// Perform additional tasks before new value becomes active
	// note: no update for SCROLL_CONTROL_AY1, SCROLL_CONTROL_BY1
	switch (reg) {
		case SCREEN_MODE_0:
		case SCREEN_MODE_1:
			// TODO verify this on real V9990
			syncAtNextLine(V9990_SET_MODE, time);
			break;
		case PALETTE_CONTROL:
			renderer->setColorMode(getColorMode(val), time);
			break;
		case BACK_DROP_COLOR:
			renderer->updateBackgroundColor(val & 63, time);
			break;
		case SCROLL_CONTROL_AY0:
			renderer->updateScrollAYLow(time);
			break;
		case SCROLL_CONTROL_BY0:
			renderer->updateScrollBYLow(time);
			break;
		case SCROLL_CONTROL_AX0:
		case SCROLL_CONTROL_AX1:
			renderer->updateScrollAX(time);
			break;
		case SCROLL_CONTROL_BX0:
		case SCROLL_CONTROL_BX1:
			renderer->updateScrollBX(time);
			break;
		case DISPLAY_ADJUST:
			// TODO verify on real V9990: when exactly does a
			//  change in horizontal/vertical adjust take place
			break;
	}
	// commit the change
	regs[reg] = val;

	// Perform additional tasks after new value became active
	switch (reg) {
		case INTERRUPT_0:
			if (pendingIRQs & val) {
				irq.set();
			} else {
				irq.reset();
			}
			break;
		case INTERRUPT_1:
		case INTERRUPT_2:
		case INTERRUPT_3:
			scheduleHscan(time);
			break;
	}
}

void V9990::writePaletteRegister(byte reg, byte val, EmuTime::param time)
{
	switch (reg & 3) {
		case 0: val &= 0x9F; break;
		case 1: val &= 0x1F; break;
		case 2: val &= 0x1F; break;
		case 3: val  = 0x00; break;
	}
	palette[reg] = val;
	reg &= ~3;
	byte index = reg / 4;
	bool ys = isSuperimposing() && (palette[reg] & 0x80);
	renderer->updatePalette(index, palette[reg + 0] & 0x1F, palette[reg + 1],
	                        palette[reg + 2], ys, time);
	if (index == regs[BACK_DROP_COLOR]) {
		renderer->updateBackgroundColor(index, time);
	}
}

void V9990::getPalette(int index, byte& r, byte& g, byte& b, bool& ys) const
{
	r = palette[4 * index + 0] & 0x1F;
	g = palette[4 * index + 1];
	b = palette[4 * index + 2];
	ys = isSuperimposing() && (palette[4 * index + 0] & 0x80);
}

void V9990::createRenderer(EmuTime::param time)
{
	assert(!renderer.get());
	renderer.reset(RendererFactory::createV9990Renderer(*this, display));
	renderer->reset(time);
}

void V9990::frameStart(EmuTime::param time)
{
	// Update setings that are fixed at the start of a frame
	displayEnabled = (regs[CONTROL]       & 0x80) != 0;
	palTiming      = (regs[SCREEN_MODE_1] & 0x08) != 0;
	interlaced     = (regs[SCREEN_MODE_1] & 0x02) != 0;
	scrollAYHigh   = regs[SCROLL_CONTROL_AY1];
	scrollBYHigh   = regs[SCROLL_CONTROL_BY1];
	setVerticalTiming();
	status ^= 0x02; // flip EO bit

	bool newSuperimposing = (regs[CONTROL] & 0x20) && externalVideoSource;
	if (superimposing != newSuperimposing) {
		superimposing = newSuperimposing;
		renderer->updateSuperimposing(superimposing, time);
	}

	frameStartTime.reset(time);

	// schedule next VSYNC
	setSyncPoint(
		frameStartTime + V9990DisplayTiming::getUCTicksPerFrame(palTiming),
		V9990_VSYNC);

	// schedule DISPLAY_START
	setSyncPoint(
	    frameStartTime + getTopBorder() * V9990DisplayTiming::UC_TICKS_PER_LINE,
	    V9990_DISPLAY_START);

	// schedule VSCAN
	setSyncPoint(
	    frameStartTime + getBottomBorder() * V9990DisplayTiming::UC_TICKS_PER_LINE,
	    V9990_VSCAN);

	renderer->frameStart(time);
}

void V9990::raiseIRQ(IRQType irqType)
{
	pendingIRQs |= irqType;
	if (pendingIRQs & regs[INTERRUPT_0]) {
		irq.set();
	}
}

void V9990::setHorizontalTiming()
{
	switch (mode) {
	case P1: case P2:
	case B1: case B3: case B7:
		horTiming = &V9990DisplayTiming::lineMCLK;
		break;
	case B0: case B2: case B4:
		horTiming = &V9990DisplayTiming::lineXTAL;
	case B5: case B6:
		break;
	default:
		UNREACHABLE;
	}
}

void V9990::setVerticalTiming()
{
	switch (mode) {
	case P1: case P2:
	case B1: case B3: case B7:
		verTiming = isPalTiming()
		          ? &V9990DisplayTiming::displayPAL_MCLK
		          : &V9990DisplayTiming::displayNTSC_MCLK;
		break;
	case B0: case B2: case B4:
		verTiming = isPalTiming()
		          ? &V9990DisplayTiming::displayPAL_XTAL
		          : &V9990DisplayTiming::displayNTSC_XTAL;
	case B5: case B6:
		break;
	default:
		UNREACHABLE;
	}
}

V9990ColorMode V9990::getColorMode(byte pal_ctrl) const
{
	V9990ColorMode mode = INVALID_COLOR_MODE;

	if (!(regs[SCREEN_MODE_0] & 0x80)) {
		mode = BP4;
	} else {
		switch (regs[SCREEN_MODE_0] & 0x03) {
			case 0x00: mode = BP2; break;
			case 0x01: mode = BP4; break;
			case 0x02:
				switch (pal_ctrl & 0xC0) {
					case 0x00: mode = BP6; break;
					case 0x40: mode = BD8; break;
					case 0x80: mode = BYJK; break;
					case 0xC0: mode = BYUV; break;
					default: UNREACHABLE;
				}
				break;
			case 0x03: mode = BD16; break;
			default: UNREACHABLE;
		}
	}

	// TODO Check
	if (mode == INVALID_COLOR_MODE) mode = BP4;
	return mode;
}

V9990ColorMode V9990::getColorMode() const
{
	return getColorMode(regs[PALETTE_CONTROL]);
}

void V9990::calcDisplayMode()
{
	mode = INVALID_DISPLAY_MODE;
	switch (regs[SCREEN_MODE_0] & 0xC0) {
		case 0x00:
			mode = P1;
			break;
		case 0x40:
			mode = P2;
			break;
		case 0x80:
			if(status & 0x04) { // MCLK timing
				switch(regs[SCREEN_MODE_0] & 0x30) {
				case 0x00: mode = B0; break;
				case 0x10: mode = B2; break;
				case 0x20: mode = B4; break;
				case 0x30: mode = INVALID_DISPLAY_MODE; break;
				default: UNREACHABLE;
				}
			} else { // XTAL1 timing
				switch(regs[SCREEN_MODE_0] & 0x30) {
				case 0x00: mode = B1; break;
				case 0x10: mode = B3; break;
				case 0x20: mode = B7; break;
				case 0x30: mode = INVALID_DISPLAY_MODE; break;
				}
			}
			break;
		case 0xC0:
			mode = INVALID_DISPLAY_MODE;
			break;
	}

	// TODO Check
	if (mode == INVALID_DISPLAY_MODE) mode = P1;

	setHorizontalTiming();
}

void V9990::scheduleHscan(EmuTime::param time)
{
	// remove pending HSCAN, if any
	if (hScanSyncTime > time) {
		removeSyncPoint(V9990_HSCAN);
		hScanSyncTime = time;
	}

	if (pendingIRQs & HOR_IRQ) {
		// flag already set, no need to schedule
		return;
	}

	int ticks = frameStartTime.getTicksTill_fast(time);
	int offset;
	if (regs[INTERRUPT_2] & 0x80) {
		// every line
		offset = ticks - (ticks % V9990DisplayTiming::UC_TICKS_PER_LINE);
	} else {
		int line = regs[INTERRUPT_1] + 256 * (regs[INTERRUPT_2] & 3) +
		           getTopBorder();
		offset = line * V9990DisplayTiming::UC_TICKS_PER_LINE;
	}
	int mult = (status & 0x04) ? 3 : 2; // MCLK / XTAL1
	offset += (regs[INTERRUPT_3] & 0x0F) * 64 * mult;
	if (offset <= ticks) {
		offset += V9990DisplayTiming::getUCTicksPerFrame(palTiming);
	}

	hScanSyncTime = frameStartTime + offset;
	setSyncPoint(hScanSyncTime, V9990_HSCAN);
}

static enum_string<V9990DisplayMode> displayModeInfo[] = {
	{ "INVALID", INVALID_DISPLAY_MODE },
	{ "P1", P1 }, { "P2", P2 },
	{ "B0", B0 }, { "B1", B1 }, { "B2", B2 }, { "B3", B3 },
	{ "B4", B4 }, { "B5", B5 }, { "B6", B6 }, { "B7", B7 }
};
SERIALIZE_ENUM(V9990DisplayMode, displayModeInfo);

// version 1:  initial version
// version 2:  added systemReset
template<typename Archive>
void V9990::serialize(Archive& ar, unsigned version)
{
	ar.template serializeBase<MSXDevice>(*this);
	ar.template serializeBase<Schedulable>(*this);

	ar.serialize("vram", *vram);
	ar.serialize("cmdEngine", *cmdEngine);
	ar.serialize("irq", irq);
	ar.serialize("frameStartTime", frameStartTime);
	ar.serialize("hScanSyncTime", hScanSyncTime);
	ar.serialize("displayMode", mode);
	ar.serialize_blob("palette", palette, sizeof(palette));
	ar.serialize("status", status);
	ar.serialize("pendingIRQs", pendingIRQs);
	ar.serialize_blob("registers", regs, sizeof(regs));
	ar.serialize("regSelect", regSelect);
	ar.serialize("palTiming", palTiming);
	ar.serialize("interlaced", interlaced);
	ar.serialize("isDisplayArea", isDisplayArea);
	ar.serialize("displayEnabled", displayEnabled);
	ar.serialize("scrollAYHigh", scrollAYHigh);
	ar.serialize("scrollBYHigh", scrollBYHigh);

	if (ar.versionBelow(version, 2)) {
		systemReset = false;
	} else {
		ar.serialize("systemReset", systemReset);
	}
	// No need to serialize 'externalVideoSource', it will be restored when
	// the external peripheral (e.g. Video9000) is de-serialized.
	// TODO should 'superimposing' be serialized? It can't be recalculated
	// from register values (it depends on the register values at the start
	// of this frame). But it will be correct at the start of the next
	// frame. Good enough?

	if (ar.isLoader()) {
		// TODO This uses 'mode' to calculate 'horTiming' and
		//      'verTiming'. Are these always in sync? Or can for
		//      example one change at any time and the other only
		//      at start of frame (or next line)? Does this matter?
		setHorizontalTiming();
		setVerticalTiming();

		renderer->reset(Schedulable::getCurrentTime());
	}
}
INSTANTIATE_SERIALIZE_METHODS(V9990);
REGISTER_MSXDEVICE(V9990, "V9990");

} // namespace openmsx
