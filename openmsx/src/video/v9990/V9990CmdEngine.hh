// $Id: V9990CmdEngine.hh 11690 2010-09-27 17:15:17Z m9710797 $

#ifndef V9990CMDENGINE_HH
#define V9990CMDENGINE_HH

#include "V9990DisplayTiming.hh"
#include "Observer.hh"
#include "Clock.hh"
#include "openmsx.hh"
#include "noncopyable.hh"

namespace openmsx {

class V9990;
class V9990VRAM;
class Setting;
class RenderSettings;
class BooleanSetting;

/** Command engine.
  */
class V9990CmdEngine : private Observer<Setting>, private noncopyable
{
public:
	// status bits
	static const byte TR = 0x80;
	static const byte BD = 0x10;
	static const byte CE = 0x01;

	V9990CmdEngine(V9990& vdp, EmuTime::param time,
	               RenderSettings& settings);
	~V9990CmdEngine();

	/** Re-initialise the command engine's state
	  * @param time   Moment in emulated time the reset occurs
	  */
	void reset(EmuTime::param time);

	/** Synchronises the command engine with the V9990
	  * @param time The moment in emulated time to sync to.
	  */
	inline void sync(EmuTime::param time) {
		if (currentCommand) currentCommand->execute(time);
	}

	/** Set a value to one of the command registers
	  */
	void setCmdReg(byte reg, byte val, EmuTime::param time);

	/** set the data byte
	  */
	void setCmdData(byte value, EmuTime::param time);

	/** read the command data byte
	  */
	byte getCmdData(EmuTime::param time);

	/** read the command data byte (without side-effects)
	  */
	byte peekCmdData(EmuTime::param time);

	/** Get command engine related status bits
	  *  - TR command data transfer ready (bit 7)
	  *  - BD border color detect         (bit 4)
	  *  - CE command being executed      (bit 0)
	  */
	byte getStatus(EmuTime::param time) {
		// note: used for both normal and debug read
		sync(time);
		return status;
	}

	word getBorderX(EmuTime::param time) {
		// note: used for both normal and debug read
		sync(time);
		return borderX;
	}

	template<typename Archive>
	void serialize(Archive& ar, unsigned version);

private:
	class V9990P1 {
	public:
		typedef byte Type;
		static const word BITS_PER_PIXEL  = 4;
		static const word PIXELS_PER_BYTE = 2;
		static inline unsigned getPitch(unsigned width);
		static inline unsigned addressOf(unsigned x, unsigned y, unsigned pitch);
		static inline byte point(V9990VRAM& vram,
		                         unsigned x, unsigned y, unsigned pitch);
		static inline byte shift(byte value, unsigned fromX, unsigned toX);
		static inline byte shiftMask(unsigned x);
		static inline const byte* getLogOpLUT(byte op);
		static inline byte logOp(const byte* lut, byte src, byte dst);
		static inline void pset(
			V9990VRAM& vram, unsigned x, unsigned y, unsigned pitch,
			byte srcColor, word mask, const byte* lut, byte op);
		static inline void psetColor(
			V9990VRAM& vram, unsigned x, unsigned y, unsigned pitch,
			word color, word mask, const byte* lut, byte op);
	};

	class V9990P2 {
	public:
		typedef byte Type;
		static const word BITS_PER_PIXEL  = 4;
		static const word PIXELS_PER_BYTE = 2;
		static inline unsigned getPitch(unsigned width);
		static inline unsigned addressOf(unsigned x, unsigned y, unsigned pitch);
		static inline byte point(V9990VRAM& vram,
		                         unsigned x, unsigned y, unsigned pitch);
		static inline byte shift(byte value, unsigned fromX, unsigned toX);
		static inline byte shiftMask(unsigned x);
		static inline const byte* getLogOpLUT(byte op);
		static inline byte logOp(const byte* lut, byte src, byte dst);
		static inline void pset(
			V9990VRAM& vram, unsigned x, unsigned y, unsigned pitch,
			byte srcColor, word mask, const byte* lut, byte op);
		static inline void psetColor(
			V9990VRAM& vram, unsigned x, unsigned y, unsigned pitch,
			word color, word mask, const byte* lut, byte op);
	};

	class V9990Bpp2 {
	public:
		typedef byte Type;
		static const word BITS_PER_PIXEL  = 2;
		static const word PIXELS_PER_BYTE = 4;
		static inline unsigned getPitch(unsigned width);
		static inline unsigned addressOf(unsigned x, unsigned y, unsigned pitch);
		static inline byte point(V9990VRAM& vram,
		                         unsigned x, unsigned y, unsigned pitch);
		static inline byte shift(byte value, unsigned fromX, unsigned toX);
		static inline byte shiftMask(unsigned x);
		static inline const byte* getLogOpLUT(byte op);
		static inline byte logOp(const byte* lut, byte src, byte dst);
		static inline void pset(
			V9990VRAM& vram, unsigned x, unsigned y, unsigned pitch,
			byte srcColor, word mask, const byte* lut, byte op);
		static inline void psetColor(
			V9990VRAM& vram, unsigned x, unsigned y, unsigned pitch,
			word color, word mask, const byte* lut, byte op);
	};

	class V9990Bpp4 {
	public:
		typedef byte Type;
		static const word BITS_PER_PIXEL  = 4;
		static const word PIXELS_PER_BYTE = 2;
		static inline unsigned getPitch(unsigned width);
		static inline unsigned addressOf(unsigned x, unsigned y, unsigned pitch);
		static inline byte point(V9990VRAM& vram,
		                         unsigned x, unsigned y, unsigned pitch);
		static inline byte shift(byte value, unsigned fromX, unsigned toX);
		static inline byte shiftMask(unsigned x);
		static inline const byte* getLogOpLUT(byte op);
		static inline byte logOp(const byte* lut, byte src, byte dst);
		static inline void pset(
			V9990VRAM& vram, unsigned x, unsigned y, unsigned pitch,
			byte srcColor, word mask, const byte* lut, byte op);
		static inline void psetColor(
			V9990VRAM& vram, unsigned x, unsigned y, unsigned pitch,
			word color, word mask, const byte* lut, byte op);
	};

	class V9990Bpp8 {
	public:
		typedef byte Type;
		static const word BITS_PER_PIXEL  = 8;
		static const word PIXELS_PER_BYTE = 1;
		static inline unsigned getPitch(unsigned width);
		static inline unsigned addressOf(unsigned x, unsigned y, unsigned pitch);
		static inline byte point(V9990VRAM& vram,
		                         unsigned x, unsigned y, unsigned pitch);
		static inline byte shift(byte value, unsigned fromX, unsigned toX);
		static inline byte shiftMask(unsigned x);
		static inline const byte* getLogOpLUT(byte op);
		static inline byte logOp(const byte* lut, byte src, byte dst);
		static inline void pset(
			V9990VRAM& vram, unsigned x, unsigned y, unsigned pitch,
			byte srcColor, word mask, const byte* lut, byte op);
		static inline void psetColor(
			V9990VRAM& vram, unsigned x, unsigned y, unsigned pitch,
			word color, word mask, const byte* lut, byte op);
	};

	class V9990Bpp16 {
	public:
		typedef word Type;
		static const word BITS_PER_PIXEL  = 16;
		static const word PIXELS_PER_BYTE = 0;
		static inline unsigned getPitch(unsigned width);
		static inline unsigned addressOf(unsigned x, unsigned y, unsigned pitch);
		static inline word point(V9990VRAM& vram,
		                         unsigned x, unsigned y, unsigned pitch);
		static inline word shift(word value, unsigned fromX, unsigned toX);
		static inline word shiftMask(unsigned x);
		static inline const byte* getLogOpLUT(byte op);
		static inline word logOp(const byte* lut, word src, word dst, bool transp);
		static inline void pset(
			V9990VRAM& vram, unsigned x, unsigned y, unsigned pitch,
			word srcColor, word mask, const byte* lut, byte op);
		static inline void psetColor(
			V9990VRAM& vram, unsigned x, unsigned y, unsigned pitch,
			word color, word mask, const byte* lut, byte op);
	};

	/** This is an abstract base class for V9990 commands
	  */
	class V9990Cmd {
	public:
		V9990Cmd(V9990CmdEngine& engine, V9990VRAM& vram);
		virtual ~V9990Cmd();

		virtual void start(EmuTime::param time) = 0;
		virtual void execute(EmuTime::param time) = 0;

	protected:
		V9990CmdEngine& engine;
		V9990VRAM&      vram;
	};

	class CmdSTOP: public V9990Cmd {
	public:
		CmdSTOP(V9990CmdEngine& engine, V9990VRAM& vram);
		virtual void start(EmuTime::param time);
		virtual void execute(EmuTime::param time);
	};

	template <class Mode>
	class CmdLMMC: public V9990Cmd {
	public:
		CmdLMMC(V9990CmdEngine& engine, V9990VRAM& vram);
		virtual void start(EmuTime::param time);
		virtual void execute(EmuTime::param time);
	};

	template <class Mode>
	class CmdLMMV: public V9990Cmd {
	public:
		CmdLMMV(V9990CmdEngine& engine, V9990VRAM& vram);
		virtual void start(EmuTime::param time);
		virtual void execute(EmuTime::param time);
	};

	template <class Mode>
	class CmdLMCM: public V9990Cmd {
	public:
		CmdLMCM(V9990CmdEngine& engine, V9990VRAM& vram);
		virtual void start(EmuTime::param time);
		virtual void execute(EmuTime::param time);
	private:
		typename Mode::Type getData(EmuTime::param time);
	};

	template <class Mode>
	class CmdLMMM: public V9990Cmd {
	public:
		CmdLMMM(V9990CmdEngine& engine, V9990VRAM& vram);
		virtual void start(EmuTime::param time);
		virtual void execute(EmuTime::param time);
	};

	template <class Mode>
	class CmdCMMC: public V9990Cmd {
	public:
		CmdCMMC(V9990CmdEngine& engine, V9990VRAM& vram);
		virtual void start(EmuTime::param time);
		virtual void execute(EmuTime::param time);
	};

	template <class Mode>
	class CmdCMMK: public V9990Cmd {
	public:
		CmdCMMK(V9990CmdEngine& engine, V9990VRAM& vram);
		virtual void start(EmuTime::param time);
		virtual void execute(EmuTime::param time);
	};

	template <class Mode>
	class CmdCMMM: public V9990Cmd {
	public:
		CmdCMMM(V9990CmdEngine& engine, V9990VRAM& vram);
		virtual void start(EmuTime::param time);
		virtual void execute(EmuTime::param time);
	};

	template <class Mode>
	class CmdBMXL: public V9990Cmd {
	public:
		CmdBMXL(V9990CmdEngine& engine, V9990VRAM& vram);
		virtual void start(EmuTime::param time);
		virtual void execute(EmuTime::param time);
	};

	template <class Mode>
	class CmdBMLX: public V9990Cmd {
	public:
		CmdBMLX(V9990CmdEngine& engine, V9990VRAM& vram);
		virtual void start(EmuTime::param time);
		virtual void execute(EmuTime::param time);
	};

	template <class Mode>
	class CmdBMLL: public V9990Cmd {
	public:
		CmdBMLL(V9990CmdEngine& engine, V9990VRAM& vram);
		virtual void start(EmuTime::param time);
		virtual void execute(EmuTime::param time);
	};

	template <class Mode>
	class CmdLINE: public V9990Cmd {
	public:
		CmdLINE(V9990CmdEngine& engine, V9990VRAM& vram);
		virtual void start(EmuTime::param time);
		virtual void execute(EmuTime::param time);
	};

	template <class Mode>
	class CmdSRCH: public V9990Cmd {
	public:
		CmdSRCH(V9990CmdEngine& engine, V9990VRAM& vram);
		virtual void start(EmuTime::param time);
		virtual void execute(EmuTime::param time);
	};

	template <class Mode>
	class CmdPOINT: public V9990Cmd {
	public:
		CmdPOINT(V9990CmdEngine& engine, V9990VRAM& vram);
		virtual void start(EmuTime::param time);
		virtual void execute(EmuTime::param time);
	};

	template <class Mode>
	class CmdPSET: public V9990Cmd {
	public:
		CmdPSET(V9990CmdEngine& engine, V9990VRAM& vram);
		virtual void start(EmuTime::param time);
		virtual void execute(EmuTime::param time);
	};

	template <class Mode>
	class CmdADVN: public V9990Cmd {
	public:
		CmdADVN(V9990CmdEngine& engine, V9990VRAM& vram);
		virtual void start(EmuTime::param time);
		virtual void execute(EmuTime::param time);
	};

	RenderSettings& settings;

	/** Only call reportV9990Command() when this setting is turned on
	  */
	BooleanSetting* cmdTraceSetting;

	/** V9990 VDP this engine belongs to
	  */
	V9990& vdp;

	/** All commands
	  */
	V9990Cmd* commands[16][6];

	/** The current command
	  */
	V9990Cmd* currentCommand;
	Clock<V9990DisplayTiming::UC_TICKS_PER_SECOND> clock;

	/** VRAM read/write address for various commands
	  */
	unsigned srcAddress;
	unsigned dstAddress;
	unsigned nbBytes;

	/** The X coord of a border detected by SRCH
	 */
	word borderX;

	/** counters
	  */
	word ASX, ADX, ANX, ANY;

	/** Command parameters
	  */
	word SX, SY, DX, DY, NX, NY;
	word WM, fgCol, bgCol;
	byte ARG, LOG, CMD;

	/** Status bits
	 */
	byte status;

	/** Data byte to transfer between V9990 and CPU
	  */
	byte data;

	/** Bit counter for CMMx commands
	  */
	byte bitsLeft;

	/** Partial data for LMMC command
	  */
	byte partial;

	/** Should command end after next getCmdData().
	 */
	bool endAfterRead;

	/** Real command timing or instantaneous (broken) timing
	 */
	bool brokenTiming;

	/** Create the engines for a given command.
	  * For each bitdepth, a separate engine is created.
	  */
	template <template <class Mode> class Command>
	void createEngines(int cmd);

	/** The running command is complete. Perform neccessary clean-up actions.
	  */
	void cmdReady(EmuTime::param time);

	/** For debugging: Print the info about the current command.
	  */
	void reportV9990Command();

	// Observer<Setting>
	virtual void update(const Setting& setting);

	void setCurrentCommand();
	unsigned getTiming(const unsigned table[4][3][4]) const;

	inline unsigned getWrappedNX() const {
		return NX ? NX : 2048;
	}
	inline unsigned getWrappedNY() const {
		return NY ? NY : 4096;
	}
};

} // namespace openmsx

#endif
