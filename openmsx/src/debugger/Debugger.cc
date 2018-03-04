// $Id: Debugger.cc 12808 2012-08-13 19:27:30Z m9710797 $

#include "Debugger.hh"
#include "Debuggable.hh"
#include "Probe.hh"
#include "ProbeBreakPoint.hh"
#include "MSXMotherBoard.hh"
#include "Reactor.hh"
#include "MSXCPU.hh"
#include "MSXCPUInterface.hh"
#include "BreakPoint.hh"
#include "DebugCondition.hh"
#include "MSXWatchIODevice.hh"
#include "TclObject.hh"
#include "RecordedCommand.hh"
#include "CommandException.hh"
#include "MemBuffer.hh"
#include "StringOp.hh"
#include "shared_ptr.hh"
#include "unreachable.hh"
#include <cassert>
#include <memory>

using std::map;
using std::set;
using std::string;
using std::vector;
using std::auto_ptr;

namespace openmsx {

typedef MSXCPUInterface::BreakPoints BreakPoints;

class DebugCmd : public RecordedCommand
{
public:
	DebugCmd(CommandController& commandController,
	         StateChangeDistributor& stateChangeDistributor,
	         Scheduler& scheduler, GlobalCliComm& cliComm,
	         Debugger& debugger);
	virtual bool needRecord(const vector<TclObject>& tokens) const;
	virtual void execute(const vector<TclObject>& tokens,
	                     TclObject& result, EmuTime::param time);
	virtual string help(const vector<string>& tokens) const;
	virtual void tabCompletion(vector<string>& tokens) const;

private:
	void list(TclObject& result);
	void desc(const vector<TclObject>& tokens,
	          TclObject& result);
	void size(const vector<TclObject>& tokens,
	          TclObject& result);
	void read(const vector<TclObject>& tokens,
	          TclObject& result);
	void readBlock(const vector<TclObject>& tokens,
	               TclObject& result);
	void write(const vector<TclObject>& tokens,
	           TclObject& result);
	void writeBlock(const vector<TclObject>& tokens,
	                TclObject& result);
	void setBreakPoint(const vector<TclObject>& tokens,
	                   TclObject& result);
	void removeBreakPoint(const vector<TclObject>& tokens,
	                      TclObject& result);
	void listBreakPoints(const vector<TclObject>& tokens,
	                     TclObject& result);
	set<string> getBreakPointIdsAsStringSet() const;
	set<string> getWatchPointIdsAsStringSet() const;
	set<string> getConditionIdsAsStringSet() const;
	void setWatchPoint(const vector<TclObject>& tokens,
	                   TclObject& result);
	void removeWatchPoint(const vector<TclObject>& tokens,
	                      TclObject& result);
	void listWatchPoints(const vector<TclObject>& tokens,
	                     TclObject& result);
	void setCondition(const vector<TclObject>& tokens,
	                  TclObject& result);
	void removeCondition(const vector<TclObject>& tokens,
	                     TclObject& result);
	void listConditions(const vector<TclObject>& tokens,
	                    TclObject& result);
	void probe(const vector<TclObject>& tokens,
	           TclObject& result);
	void probeList(const vector<TclObject>& tokens,
	               TclObject& result);
	void probeDesc(const vector<TclObject>& tokens,
	               TclObject& result);
	void probeRead(const vector<TclObject>& tokens,
	               TclObject& result);
	void probeSetBreakPoint(const vector<TclObject>& tokens,
	                        TclObject& result);
	void probeRemoveBreakPoint(const vector<TclObject>& tokens,
	                           TclObject& result);
	void probeListBreakPoints(const vector<TclObject>& tokens,
	                          TclObject& result);

	GlobalCliComm& cliComm;
	Debugger& debugger;
};


Debugger::Debugger(MSXMotherBoard& motherBoard_)
	: motherBoard(motherBoard_)
	, debugCmd(new DebugCmd(motherBoard.getCommandController(),
	                        motherBoard.getStateChangeDistributor(),
	                        motherBoard.getScheduler(),
	                        motherBoard.getReactor().getGlobalCliComm(), *this))
	, cpu(0)
{
}

Debugger::~Debugger()
{
	for (ProbeBreakPoints::const_iterator it = probeBreakPoints.begin();
	     it != probeBreakPoints.end(); ++it) {
		delete *it;
	}

	assert(!cpu);
	assert(debuggables.empty());
}

void Debugger::setCPU(MSXCPU* cpu_)
{
	cpu = cpu_;
}

void Debugger::registerDebuggable(string_ref name, Debuggable& debuggable)
{
	assert(debuggables.find(name) == debuggables.end());
	debuggables[name] = &debuggable;
}

void Debugger::unregisterDebuggable(string_ref name, Debuggable& debuggable)
{
	(void)debuggable;
	Debuggables::iterator it = debuggables.find(name);
	assert(it != debuggables.end() && (it->second == &debuggable));
	debuggables.erase(it);
}

Debuggable* Debugger::findDebuggable(string_ref name)
{
	Debuggables::iterator it = debuggables.find(name);
	return (it != debuggables.end()) ? it->second : NULL;
}

Debuggable& Debugger::getDebuggable(string_ref name)
{
	Debuggable* result = findDebuggable(name);
	if (!result) {
		throw CommandException("No such debuggable: " + name);
	}
	return *result;
}

void Debugger::getDebuggables(set<string>& result) const
{
	for (Debuggables::const_iterator it = debuggables.begin();
	     it != debuggables.end(); ++it) {
		result.insert(it->first().str());
	}
}


void Debugger::registerProbe(string_ref name, ProbeBase& probe)
{
	assert(probes.find(name) == probes.end());
	probes[name] = &probe;
}

void Debugger::unregisterProbe(string_ref name, ProbeBase& probe)
{
	(void)probe;
	Probes::iterator it = probes.find(name);
	assert(it != probes.end() && (it->second == &probe));
	probes.erase(it);
}

ProbeBase* Debugger::findProbe(string_ref name)
{
	Probes::iterator it = probes.find(name);
	return (it != probes.end()) ? it->second : NULL;
}

ProbeBase& Debugger::getProbe(string_ref name)
{
	ProbeBase* result = findProbe(name);
	if (!result) {
		throw CommandException("No such probe: " + name);
	}
	return *result;
}

void Debugger::getProbes(std::set<std::string>& result) const
{
	for (Probes::const_iterator it = probes.begin();
	     it != probes.end(); ++it) {
		result.insert(it->first().str());
	}
}


unsigned Debugger::insertProbeBreakPoint(
	TclObject command, TclObject condition,
	ProbeBase& probe, unsigned newId /*= -1*/)
{
	ProbeBreakPoint* bp = new ProbeBreakPoint(
		motherBoard.getReactor().getGlobalCliComm(),
		command, condition, *this, probe, newId);
	probeBreakPoints.push_back(bp);
	return bp->getId();
}

void Debugger::removeProbeBreakPoint(string_ref name)
{
	if (name.starts_with("pp#")) {
		// remove by id
		unsigned id = stoi(name.substr(3));
		for (ProbeBreakPoints::iterator it = probeBreakPoints.begin();
		     it != probeBreakPoints.end(); ++it) {
			if ((*it)->getId() == id) {
				delete *it;
				probeBreakPoints.erase(it);
				return;
			}
		}
		throw CommandException("No such breakpoint: " + name);
	} else {
		// remove by probe, only works for unconditional bp
		for (ProbeBreakPoints::iterator it = probeBreakPoints.begin();
		     it != probeBreakPoints.end(); ++it) {
			if ((*it)->getProbe().getName() == name) {
				delete *it;
				probeBreakPoints.erase(it);
				return;
			}
		}
		throw CommandException(
			"No (unconditional) breakpoint for probe: " + name);
	}
}

void Debugger::removeProbeBreakPoint(ProbeBreakPoint& bp)
{
	ProbeBreakPoints::iterator it =
		find(probeBreakPoints.begin(), probeBreakPoints.end(), &bp);
	assert(it != probeBreakPoints.end());
	delete *it;
	probeBreakPoints.erase(it);
}

unsigned Debugger::setWatchPoint(TclObject command, TclObject condition,
                                 WatchPoint::Type type,
                                 unsigned beginAddr, unsigned endAddr,
                                 unsigned newId /*= -1*/)
{
	shared_ptr<WatchPoint> wp;
	if ((type == WatchPoint::READ_IO) || (type == WatchPoint::WRITE_IO)) {
		wp.reset(new WatchIO(motherBoard, type, beginAddr, endAddr,
		                     command, condition, newId));
	} else {
		wp.reset(new WatchPoint(motherBoard.getReactor().getGlobalCliComm(),
			command, condition, type, beginAddr, endAddr, newId));
	}
	motherBoard.getCPUInterface().setWatchPoint(wp);
	return wp->getId();
}

void Debugger::transfer(Debugger& other)
{
	// Copy watchpoints to new machine.
	assert(motherBoard.getCPUInterface().getWatchPoints().empty());
	const MSXCPUInterface::WatchPoints& watchPoints =
		other.motherBoard.getCPUInterface().getWatchPoints();
	for (MSXCPUInterface::WatchPoints::const_iterator it =
	     watchPoints.begin(); it != watchPoints.end(); ++it) {
		const WatchPoint& wp = **it;
		setWatchPoint(wp.getCommandObj(), wp.getConditionObj(),
		              wp.getType(), wp.getBeginAddress(),
		              wp.getEndAddress(), wp.getId());
	}

	// Copy probes to new machine.
	assert(probeBreakPoints.empty());
	for (ProbeBreakPoints::const_iterator it = other.probeBreakPoints.begin();
	     it != other.probeBreakPoints.end(); ++it) {
		const ProbeBreakPoint& bp = **it;
		if (ProbeBase* probe = findProbe(bp.getProbe().getName())) {
			insertProbeBreakPoint(bp.getCommandObj(),
			                      bp.getConditionObj(),
			                      *probe, bp.getId());
		}
	}

	// Breakpoints and conditions are (currently) global, so no need to
	// copy those.
}


// class DebugCmd

static word getAddress(const vector<TclObject>& tokens)
{
	if (tokens.size() < 3) {
		throw CommandException("Missing argument");
	}
	unsigned addr = tokens[2].getInt();
	if (addr >= 0x10000) {
		throw CommandException("Invalid address");
	}
	return addr;
}

DebugCmd::DebugCmd(CommandController& commandController,
                   StateChangeDistributor& stateChangeDistributor,
                   Scheduler& scheduler, GlobalCliComm& cliComm_,
                   Debugger& debugger_)
	: RecordedCommand(commandController, stateChangeDistributor,
	                  scheduler, "debug")
	, cliComm(cliComm_)
	, debugger(debugger_)
{
}

bool DebugCmd::needRecord(const vector<TclObject>& tokens) const
{
	// Note: it's crucial for security that only the write and write_block
	// subcommands are recorded and replayed. The 'set_bp' command for
	// example would allow to set a callback that can execute arbitrary Tcl
	// code. See comments in RecordedCommand for more details.
	if (tokens.size() < 2) return false;
	string_ref subCmd = tokens[1].getString();
	return (subCmd == "write") || (subCmd == "write_block");
}

void DebugCmd::execute(const vector<TclObject>& tokens,
                       TclObject& result, EmuTime::param /*time*/)
{
	if (tokens.size() < 2) {
		throw CommandException("Missing argument");
	}
	string_ref subCmd = tokens[1].getString();
	if (subCmd == "read") {
		read(tokens, result);
	} else if (subCmd == "read_block") {
		readBlock(tokens, result);
	} else if (subCmd == "write") {
		write(tokens, result);
	} else if (subCmd == "write_block") {
		writeBlock(tokens, result);
	} else if (subCmd == "size") {
		size(tokens, result);
	} else if (subCmd == "desc") {
		desc(tokens, result);
	} else if (subCmd == "list") {
		list(result);
	} else if (subCmd == "step") {
		debugger.motherBoard.getCPUInterface().doStep();
	} else if (subCmd == "cont") {
		debugger.motherBoard.getCPUInterface().doContinue();
	} else if (subCmd == "disasm") {
		debugger.cpu->disasmCommand(tokens, result);
	} else if (subCmd == "break") {
		debugger.motherBoard.getCPUInterface().doBreak();
	} else if (subCmd == "breaked") {
		result.setInt(debugger.motherBoard.getCPUInterface().isBreaked());
	} else if (subCmd == "set_bp") {
		setBreakPoint(tokens, result);
	} else if (subCmd == "remove_bp") {
		removeBreakPoint(tokens, result);
	} else if (subCmd == "list_bp") {
		listBreakPoints(tokens, result);
	} else if (subCmd == "set_watchpoint") {
		setWatchPoint(tokens, result);
	} else if (subCmd == "remove_watchpoint") {
		removeWatchPoint(tokens, result);
	} else if (subCmd == "list_watchpoints") {
		listWatchPoints(tokens, result);
	} else if (subCmd == "set_condition") {
		setCondition(tokens, result);
	} else if (subCmd == "remove_condition") {
		removeCondition(tokens, result);
	} else if (subCmd == "list_conditions") {
		listConditions(tokens, result);
	} else if (subCmd == "probe") {
		probe(tokens, result);
	} else {
		throw SyntaxError();
	}
}

void DebugCmd::list(TclObject& result)
{
	set<string> debuggables;
	debugger.getDebuggables(debuggables);
	result.addListElements(debuggables.begin(), debuggables.end());
}

void DebugCmd::desc(const vector<TclObject>& tokens, TclObject& result)
{
	if (tokens.size() != 3) {
		throw SyntaxError();
	}
	Debuggable& device = debugger.getDebuggable(tokens[2].getString());
	result.setString(device.getDescription());
}

void DebugCmd::size(const vector<TclObject>& tokens, TclObject& result)
{
	if (tokens.size() != 3) {
		throw SyntaxError();
	}
	Debuggable& device = debugger.getDebuggable(tokens[2].getString());
	result.setInt(device.getSize());
}

void DebugCmd::read(const vector<TclObject>& tokens, TclObject& result)
{
	if (tokens.size() != 4) {
		throw SyntaxError();
	}
	Debuggable& device = debugger.getDebuggable(tokens[2].getString());
	unsigned addr = tokens[3].getInt();
	if (addr >= device.getSize()) {
		throw CommandException("Invalid address");
	}
	result.setInt(device.read(addr));
}

void DebugCmd::readBlock(const vector<TclObject>& tokens, TclObject& result)
{
	if (tokens.size() != 5) {
		throw SyntaxError();
	}
	Debuggable& device = debugger.getDebuggable(tokens[2].getString());
	unsigned size = device.getSize();
	unsigned addr = tokens[3].getInt();
	if (addr >= size) {
		throw CommandException("Invalid address");
	}
	unsigned num = tokens[4].getInt();
	if (num > (size - addr)) {
		throw CommandException("Invalid size");
	}

	MemBuffer<byte> buf(num);
	for (unsigned i = 0; i < num; ++i) {
		buf[i] = device.read(addr + i);
	}
	result.setBinary(buf.data(), num);
}

void DebugCmd::write(const vector<TclObject>& tokens,
                               TclObject& /*result*/)
{
	if (tokens.size() != 5) {
		throw SyntaxError();
	}
	Debuggable& device = debugger.getDebuggable(tokens[2].getString());
	unsigned addr = tokens[3].getInt();
	if (addr >= device.getSize()) {
		throw CommandException("Invalid address");
	}
	unsigned value = tokens[4].getInt();
	if (value >= 256) {
		throw CommandException("Invalid value");
	}

	device.write(addr, value);
}

void DebugCmd::writeBlock(const vector<TclObject>& tokens,
                                    TclObject& /*result*/)
{
	if (tokens.size() != 5) {
		throw SyntaxError();
	}
	Debuggable& device = debugger.getDebuggable(tokens[2].getString());
	unsigned size = device.getSize();
	unsigned addr = tokens[3].getInt();
	if (addr >= size) {
		throw CommandException("Invalid address");
	}
	unsigned num;
	const byte* buf = tokens[4].getBinary(num);
	if ((num + addr) > size) {
		throw CommandException("Invalid size");
	}

	for (unsigned i = 0; i < num; ++i) {
		device.write(addr + i, static_cast<byte>(buf[i]));
	}
}

void DebugCmd::setBreakPoint(const vector<TclObject>& tokens,
                             TclObject& result)
{
	shared_ptr<BreakPoint> bp;
	TclObject command  (result.getInterpreter(), "debug break");
	TclObject condition(result.getInterpreter());
	word addr;

	switch (tokens.size()) {
	case 5: // command
		command = tokens[4];
		command.checkCommand();
		// fall-through
	case 4: // condition
		condition = tokens[3];
		if (!condition.getString().empty()) {
			condition.checkExpression();
		}
		// fall-through
	case 3: // address
		addr = getAddress(tokens);
		bp.reset(new BreakPoint(cliComm, addr, command, condition));
		break;
	default:
		if (tokens.size() < 3) {
			throw CommandException("Too few arguments.");
		} else {
			throw CommandException("Too many arguments.");
		}
	}
	result.setString(StringOp::Builder() << "bp#" << bp->getId());
	debugger.motherBoard.getCPUInterface().insertBreakPoint(bp);
}

void DebugCmd::removeBreakPoint(const vector<TclObject>& tokens,
                                          TclObject& /*result*/)
{
	if (tokens.size() != 3) {
		throw SyntaxError();
	}
	MSXCPUInterface& interface = debugger.motherBoard.getCPUInterface();
	const BreakPoints& breakPoints = interface.getBreakPoints();

	string_ref tmp = tokens[2].getString();
	if (tmp.starts_with("bp#")) {
		// remove by id
		unsigned id = stoi(tmp.substr(3));
		for (BreakPoints::const_iterator it = breakPoints.begin();
		     it != breakPoints.end(); ++it) {
			const BreakPoint& bp = *it->second;
			if (bp.getId() == id) {
				interface.removeBreakPoint(bp);
				return;
			}
		}
		throw CommandException("No such breakpoint: " + tmp);
	} else {
		// remove by addr, only works for unconditional bp
		word addr = getAddress(tokens);
		std::pair<BreakPoints::const_iterator,
			  BreakPoints::const_iterator> range =
				breakPoints.equal_range(addr);
		for (BreakPoints::const_iterator it = range.first;
		     it != range.second; ++it) {
			const BreakPoint& bp = *it->second;
			if (bp.getCondition().empty()) {
				interface.removeBreakPoint(bp);
				return;
			}
		}
		throw CommandException(
			"No (unconditional) breakpoint at address: " + tmp);
	}
}

void DebugCmd::listBreakPoints(const vector<TclObject>& /*tokens*/,
                               TclObject& result)
{
	MSXCPUInterface& interface = debugger.motherBoard.getCPUInterface();
	const BreakPoints& breakPoints = interface.getBreakPoints();
	string res;
	for (BreakPoints::const_iterator it = breakPoints.begin();
	     it != breakPoints.end(); ++it) {
		const BreakPoint& bp = *it->second;
		TclObject line(result.getInterpreter());
		line.addListElement(StringOp::Builder() << "bp#" << bp.getId());
		line.addListElement("0x" + StringOp::toHexString(bp.getAddress(), 4));
		line.addListElement(bp.getCondition());
		line.addListElement(bp.getCommand());
		res += line.getString() + '\n';
	}
	result.setString(res);
}


void DebugCmd::setWatchPoint(const vector<TclObject>& tokens,
                             TclObject& result)
{
	TclObject command  (result.getInterpreter(), "debug break");
	TclObject condition(result.getInterpreter());
	unsigned beginAddr, endAddr;
	WatchPoint::Type type;

	switch (tokens.size()) {
	case 6: // command
		command = tokens[5];
		command.checkCommand();
		// fall-through
	case 5: // condition
		condition = tokens[4];
		if (!condition.getString().empty()) {
			condition.checkExpression();
		}
		// fall-through
	case 4: { // address + type
		string_ref typeStr = tokens[2].getString();
		unsigned max;
		if (typeStr == "read_io") {
			type = WatchPoint::READ_IO;
			max = 0x100;
		} else if (typeStr == "write_io") {
			type = WatchPoint::WRITE_IO;
			max = 0x100;
		} else if (typeStr == "read_mem") {
			type = WatchPoint::READ_MEM;
			max = 0x10000;
		} else if (typeStr == "write_mem") {
			type = WatchPoint::WRITE_MEM;
			max = 0x10000;
		} else {
			throw CommandException("Invalid type: " + typeStr);
		}
		if (tokens[3].getListLength() == 2) {
			beginAddr = tokens[3].getListIndex(0).getInt();
			endAddr   = tokens[3].getListIndex(1).getInt();
			if (endAddr < beginAddr) {
				throw CommandException(
					"Not a valid range: end address may "
					"not be smaller than begin address.");
			}
		} else {
			beginAddr = endAddr = tokens[3].getInt();
		}
		if (endAddr >= max) {
			throw CommandException("Invalid address: out of range");
		}
		break;
	}
	default:
		if (tokens.size() < 4) {
			throw CommandException("Too few arguments.");
		} else {
			throw CommandException("Too many arguments.");
		}
	}
	unsigned id = debugger.setWatchPoint(
		command, condition, type, beginAddr, endAddr);
	result.setString(StringOp::Builder() << "wp#" << id);
}

void DebugCmd::removeWatchPoint(const vector<TclObject>& tokens,
                                TclObject& /*result*/)
{
	if (tokens.size() != 3) {
		throw SyntaxError();
	}
	MSXCPUInterface& interface = debugger.motherBoard.getCPUInterface();
	const MSXCPUInterface::WatchPoints& watchPoints = interface.getWatchPoints();

	string_ref tmp = tokens[2].getString();
	if (tmp.starts_with("wp#")) {
		// remove by id
		unsigned id = stoi(tmp.substr(3));
		for (MSXCPUInterface::WatchPoints::const_iterator it =
			watchPoints.begin(); it != watchPoints.end(); ++it) {
			shared_ptr<WatchPoint> wp = *it;
			if (wp->getId() == id) {
				interface.removeWatchPoint(wp);
				return;
			}
		}
	}
	throw CommandException("No such watchpoint: " + tmp);
}

void DebugCmd::listWatchPoints(const vector<TclObject>& /*tokens*/,
                               TclObject& result)
{
	MSXCPUInterface& interface = debugger.motherBoard.getCPUInterface();
	const MSXCPUInterface::WatchPoints& watchPoints = interface.getWatchPoints();
	string res;
	for (MSXCPUInterface::WatchPoints::const_iterator it = watchPoints.begin();
	     it != watchPoints.end(); ++it) {
		const WatchPoint& wp = **it;
		TclObject line(result.getInterpreter());
		line.addListElement(StringOp::Builder() << "wp#" << wp.getId());
		string type;
		switch (wp.getType()) {
		case WatchPoint::READ_IO:
			type = "read_io";
			break;
		case WatchPoint::WRITE_IO:
			type = "write_io";
			break;
		case WatchPoint::READ_MEM:
			type = "read_mem";
			break;
		case WatchPoint::WRITE_MEM:
			type = "write_mem";
			break;
		default:
			UNREACHABLE; break;
		}
		line.addListElement(type);
		unsigned beginAddr = wp.getBeginAddress();
		unsigned endAddr   = wp.getEndAddress();
		if (beginAddr == endAddr) {
			line.addListElement("0x" + StringOp::toHexString(beginAddr, 4));
		} else {
			TclObject range(result.getInterpreter());
			range.addListElement("0x" + StringOp::toHexString(beginAddr, 4));
			range.addListElement("0x" + StringOp::toHexString(endAddr,   4));
			line.addListElement(range);
		}
		line.addListElement(wp.getCondition());
		line.addListElement(wp.getCommand());
		res += line.getString() + '\n';
	}
	result.setString(res);
}


void DebugCmd::setCondition(const vector<TclObject>& tokens,
                            TclObject& result)
{
	shared_ptr<DebugCondition> dc;
	TclObject command  (result.getInterpreter(), "debug break");
	TclObject condition(result.getInterpreter());

	switch (tokens.size()) {
	case 4: // command
		command = tokens[3];
		command.checkCommand();
		// fall-through
	case 3: // condition
		condition = tokens[2];
		if (!condition.getString().empty()) {
			condition.checkExpression();
		}
		dc.reset(new DebugCondition(cliComm, command, condition));
		break;
	default:
		if (tokens.size() < 3) {
			throw CommandException("Too few arguments.");
		} else {
			throw CommandException("Too many arguments.");
		}
	}
	result.setString(StringOp::Builder() << "cond#" << dc->getId());
	debugger.motherBoard.getCPUInterface().setCondition(dc);
}

void DebugCmd::removeCondition(const vector<TclObject>& tokens,
                               TclObject& /*result*/)
{
	if (tokens.size() != 3) {
		throw SyntaxError();
	}
	MSXCPUInterface& interface = debugger.motherBoard.getCPUInterface();
	const MSXCPUInterface::Conditions& conditions = interface.getConditions();

	string_ref tmp = tokens[2].getString();
	if (tmp.starts_with("cond#")) {
		// remove by id
		unsigned id = stoi(tmp.substr(5));
		for (MSXCPUInterface::Conditions::const_iterator it =
			conditions.begin(); it != conditions.end(); ++it) {
			DebugCondition& cond = **it;
			if (cond.getId() == id) {
				interface.removeCondition(cond);
				return;
			}
		}
	}
	throw CommandException("No such condition: " + tmp);
}

void DebugCmd::listConditions(const vector<TclObject>& /*tokens*/,
                              TclObject& result)
{
	MSXCPUInterface& interface = debugger.motherBoard.getCPUInterface();
	const MSXCPUInterface::Conditions& conditions = interface.getConditions();
	string res;
	for (MSXCPUInterface::Conditions::const_iterator it = conditions.begin();
	     it != conditions.end(); ++it) {
		const DebugCondition& cond = **it;
		TclObject line(result.getInterpreter());
		line.addListElement(StringOp::Builder() << "cond#" << cond.getId());
		line.addListElement(cond.getCondition());
		line.addListElement(cond.getCommand());
		res += line.getString() + '\n';
	}
	result.setString(res);
}


void DebugCmd::probe(const vector<TclObject>& tokens,
                     TclObject& result)
{
	if (tokens.size() < 3) {
		throw CommandException("Missing argument");
	}
	string_ref subCmd = tokens[2].getString();
	if (subCmd == "list") {
		probeList(tokens, result);
	} else if (subCmd == "desc") {
		probeDesc(tokens, result);
	} else if (subCmd == "read") {
		probeRead(tokens, result);
	} else if (subCmd == "set_bp") {
		probeSetBreakPoint(tokens, result);
	} else if (subCmd == "remove_bp") {
		probeRemoveBreakPoint(tokens, result);
	} else if (subCmd == "list_bp") {
		probeListBreakPoints(tokens, result);
	} else {
		throw SyntaxError();
	}
}
void DebugCmd::probeList(const vector<TclObject>& /*tokens*/,
                         TclObject& result)
{
	set<string> probes;
	debugger.getProbes(probes);
	result.addListElements(probes.begin(), probes.end());
}
void DebugCmd::probeDesc(const vector<TclObject>& tokens,
                         TclObject& result)
{
	if (tokens.size() != 4) {
		throw SyntaxError();
	}
	ProbeBase& probe = debugger.getProbe(tokens[3].getString());
	result.setString(probe.getDescription());
}
void DebugCmd::probeRead(const vector<TclObject>& tokens,
                         TclObject& result)
{
	if (tokens.size() != 4) {
		throw SyntaxError();
	}
	ProbeBase& probe = debugger.getProbe(tokens[3].getString());
	result.setString(probe.getValue());
}
void DebugCmd::probeSetBreakPoint(const vector<TclObject>& tokens,
                                  TclObject& result)
{
	TclObject command  (result.getInterpreter(), "debug break");
	TclObject condition(result.getInterpreter());
	ProbeBase* probe;

	switch (tokens.size()) {
	case 6: // command
		command = tokens[5];
		command.checkCommand();
		// fall-through
	case 5: // condition
		condition = tokens[4];
		if (!condition.getString().empty()) {
			condition.checkExpression();
		}
		// fall-through
	case 4: { // probe
		probe = &debugger.getProbe(tokens[3].getString());
		break;
	}
	default:
		if (tokens.size() < 4) {
			throw CommandException("Too few arguments.");
		} else {
			throw CommandException("Too many arguments.");
		}
	}

	unsigned id = debugger.insertProbeBreakPoint(command, condition, *probe);
	result.setString(StringOp::Builder() << "pp#" << id);
}
void DebugCmd::probeRemoveBreakPoint(const vector<TclObject>& tokens,
                                     TclObject& /*result*/)
{
	if (tokens.size() != 4) {
		throw SyntaxError();
	}
	debugger.removeProbeBreakPoint(tokens[3].getString());
}
void DebugCmd::probeListBreakPoints(const vector<TclObject>& /*tokens*/,
                                    TclObject& result)
{
	string res;
	for (Debugger::ProbeBreakPoints::const_iterator it =
	         debugger.probeBreakPoints.begin();
	     it != debugger.probeBreakPoints.end(); ++it) {
		const ProbeBreakPoint& bp = **it;
		TclObject line(result.getInterpreter());
		line.addListElement(StringOp::Builder() << "pp#" << bp.getId());
		line.addListElement(bp.getProbe().getName());
		line.addListElement(bp.getCondition());
		line.addListElement(bp.getCommand());
		res += line.getString() + '\n';
	}
	result.setString(res);
}

string DebugCmd::help(const vector<string>& tokens) const
{
	static const string generalHelp =
		"debug <subcommand> [<arguments>]\n"
		"  Possible subcommands are:\n"
		"    list              returns a list of all debuggables\n"
		"    desc              returns a description of this debuggable\n"
		"    size              returns the size of this debuggable\n"
		"    read              read a byte from a debuggable\n"
		"    write             write a byte to a debuggable\n"
		"    read_block        read a whole block at once\n"
		"    write_block       write a whole block at once\n"
		"    set_bp            insert a new breakpoint\n"
		"    remove_bp         remove a certain breakpoint\n"
		"    list_bp           list the active breakpoints\n"
		"    set_watchpoint    insert a new watchpoint\n"
		"    remove_watchpoint remove a certain watchpoint\n"
		"    list_watchpoints  list the active watchpoints\n"
		"    set_condition     insert a new condition\n"
		"    remove_condition  remove a certain condition\n"
		"    list_conditions   list the active conditions\n"
		"    probe             probe related subcommands\n"
		"    cont              continue execution after break\n"
		"    step              execute one instruction\n"
		"    break             break CPU at current position\n"
		"    breaked           query CPU breaked status\n"
		"    disasm            disassemble instructions\n"
		"  The arguments are specific for each subcommand.\n"
		"  Type 'help debug <subcommand>' for help about a specific subcommand.\n";

	static const string listHelp =
		"debug list\n"
		"  Returns a list with the names of all 'debuggables'.\n"
		"  These names are used in other debug subcommands.\n";
	static const string descHelp =
		"debug desc <name>\n"
		"  Returns a description for the debuggable with given name.\n";
	static const string sizeHelp =
		"debug size <name>\n"
		"  Returns the size (in bytes) of the debuggable with given name.\n";
	static const string readHelp =
		"debug read <name> <addr>\n"
		"  Read a byte at offset <addr> from the given debuggable.\n"
		"  The offset must be smaller than the value returned from the "
		"'size' subcommand\n"
		"  Note that openMSX comes with a bunch of Tcl scripts that make "
		"some of the debug reads much more convenient (e.g. reading from "
		"Z80 or VDP registers). See the Console Command Reference for more "
		"details about these.\n";
	static const string writeHelp =
		"debug write <name> <addr> <val>\n"
		"  Write a byte to the given debuggable at a certain offset.\n"
		"  The offset must be smaller than the value returned from the "
		"'size' subcommand\n";
	static const string readBlockHelp =
		"debug read_block <name> <addr> <size>\n"
		"  Read a whole block at once. This is equivalent with repeated "
		"invocations of the 'read' subcommand, but using this subcommand "
		"may be faster. The result is a Tcl binary string (see Tcl manual).\n"
		"  The block is specified as size/offset in the debuggable. The "
		"complete block must fit in the debuggable (see the 'size' "
		"subcommand).\n";
	static const string writeBlockHelp =
		"debug write_block <name> <addr> <values>\n"
		"  Write a whole block at once. This is equivalent with repeated "
		"invocations of the 'write' subcommand, but using this subcommand "
		"may be faster. The <values> argument must be a Tcl binary string "
		"(see Tcl manual).\n"
		"  The block has a size and an offset in the debuggable. The "
		"complete block must fit in the debuggable (see the 'size' "
		"subcommand).\n";
	static const string setBpHelp =
		"debug set_bp <addr> [<cond>] [<cmd>]\n"
		"  Insert a new breakpoint at given address. When the CPU is about "
		"to execute the instruction at this address, execution will be "
		"breaked. At least this is the default behaviour, see next "
		"paragraphs.\n"
		"  Optionally you can specify a condition. When the CPU reaches "
		"the breakpoint this condition is evaluated, only when the condition "
		"evaluated to true execution will be breaked.\n"
		"  A condition must be specified as a Tcl expression. For example\n"
		"     debug set_bp 0xf37d {[reg C] == 0x2F}\n"
		"  This breaks on address 0xf37d but only when Z80 register C has the "
		"value 0x2F.\n"
		"  Also optionally you can specify a command that should be "
		"executed when the breakpoint is reached (and condition is true). "
		"By default this command is 'debug break'.\n"
		"  The result of this command is a breakpoint ID. This ID can "
		"later be used to remove this breakpoint again.\n";
	static const string removeBpHelp =
		"debug remove_bp <id>\n"
		"  Remove the breakpoint with given ID again. You can use the "
		"'list_bp' subcommand to see all valid IDs.\n";
	static const string listBpHelp =
		"debug list_bp\n"
		"  Lists all active breakpoints. The result is printed in 4 "
		"columns. The first column contains the breakpoint ID. The "
		"second one has the address. The third has the condition "
		"(default condition is empty). And the last column contains "
		"the command that will be executed (default is 'debug break').\n";
	static const string setWatchPointHelp =
		"debug set_watchpoint <type> <region> [<cond>] [<cmd>]\n"
		"  Insert a new watchpoint of given type on the given region, "
		"there can be an optional condition and alternative command. See "
		"the 'set_bp' subcommand for details about these last two.\n"
		"  Type must be one of the following:\n"
		"    read_io    break when CPU reads from given IO port(s)\n"
		"    write_io   break when CPU writes to given IO port(s)\n"
		"    read_mem   break when CPU reads from given memory location(s)\n"
		"    write_mem  break when CPU writes to given memory location(s)\n"
		"  Region is either a single value, this corresponds to a single "
		"memory location or IO port. Otherwise region must be a list of "
		"two values (enclosed in braces) that specify a begin and end "
		"point of a whole memory region or a range of IO ports.\n"
		"During the execution of <cmd>, the following global Tcl "
		"variables are set:\n"
		"  ::wp_last_address   this is the actual address of the mem/io "
		"read/write that triggered the watchpoint\n"
		"  ::wp_last_value     this is the actual value that was written "
		"by the mem/io write that triggered the watchpoint\n"
		"Examples:\n"
		"  debug set_watchpoint write_io 0x99 {[reg A] == 0x81}\n"
		"  debug set_watchpoint read_mem {0xfbe5 0xfbef}\n";
	static const string removeWatchPointHelp =
		"debug remove_watchpoint <id>\n"
		"  Remove the watchpoint with given ID again. You can use the "
		"'list_watchpoints' subcommand to see all valid IDs.\n";
	static const string listWatchPointsHelp =
		"debug list_watchpoints\n"
		"  Lists all active watchpoints. The result is similar to the "
		"'list_bp' subcommand, but there is an extra column (2nd column) "
		"that contains the type of the watchpoint.\n";
	static const string setCondHelp =
		"debug set_condition <cond> [<cmd>]\n"
		"  Insert a new condition. These are much like breakpoints, "
		"except that they are checked before every instruction "
		"(breakpoints are tied to a specific address).\n"
		"  Conditions will slow down simulation MUCH more than "
		"breakpoints. So only use them when you don't care about "
		"simulation speed (when you're debugging this is usually not "
		"a problem).\n"
		"  See 'help debug set_bp' for more details.\n";
	static const string removeCondHelp =
		"debug remove_condition <id>\n"
		"  Remove the condition with given ID again. You can use the "
		"'list_conditions' subcommand to see all valid IDs.\n";
	static const string listCondHelp =
		"debug list_conditions\n"
		"  Lists all active conditions. The result is similar to the "
		"'list_bp' subcommand, but without the 2nd column that would "
		"show the address.\n";
	static const string probeHelp =
		"debug probe <subcommand> [<arguments>]\n"
		"  Possible subcommands are:\n"
		"    list                             returns a list of all probes\n"
		"    desc   <probe>                   returns a description of this probe\n"
		"    read   <probe>                   returns the current value of this probe\n"
		"    set_bp <probe> [<cond>] [<cmd>]  set a breakpoint on the given probe\n"
		"    remove_bp <id>                   remove the given breakpoint\n"
		"    list_bp                          returns a list of breakpoints that are set on probes\n";
	static const string contHelp =
		"debug cont\n"
		"  Continue execution after CPU was breaked.\n";
	static const string stepHelp =
		"debug step\n"
		"  Execute one instruction. This command is only meaningful in "
		"break mode.\n";
	static const string breakHelp =
		"debug break\n"
		"  Immediately break CPU execution. When CPU was already breaked "
		"this command has no effect.\n";
	static const string breakedHelp =
		"debug breaked\n"
		"  Query the CPU breaked status. Returns '1' when CPU was "
		"breaked, '0' otherwise.\n";
	static const string disasmHelp =
		"debug disasm <addr>\n"
		"  Disassemble the instruction at the given address. The result "
		"is a Tcl list. The first element in the list contains a textual "
		"representation of the instruction, the next elements contain the "
		"bytes that make up this instruction (thus the length of the "
		"resulting list can be used to derive the number of bytes in the "
		"instruction).\n"
		"  Note that openMSX comes with a 'disasm' Tcl script that is much "
		"more convenient to use than this subcommand.";
	static const string unknownHelp =
		"Unknown subcommand, use 'help debug' to see a list of valid "
		"subcommands.\n";

	if (tokens.size() == 1) {
		return generalHelp;
	} else if (tokens[1] == "list") {
		return listHelp;
	} else if (tokens[1] == "desc") {
		return descHelp;
	} else if (tokens[1] == "size") {
		return sizeHelp;
	} else if (tokens[1] == "read") {
		return readHelp;
	} else if (tokens[1] == "write") {
		return writeHelp;
	} else if (tokens[1] == "read_block") {
		return readBlockHelp;
	} else if (tokens[1] == "write_block") {
		return writeBlockHelp;
	} else if (tokens[1] == "set_bp") {
		return setBpHelp;
	} else if (tokens[1] == "remove_bp") {
		return removeBpHelp;
	} else if (tokens[1] == "list_bp") {
		return listBpHelp;
	} else if (tokens[1] == "set_watchpoint") {
		return setWatchPointHelp;
	} else if (tokens[1] == "remove_watchpoint") {
		return removeWatchPointHelp;
	} else if (tokens[1] == "list_watchpoints") {
		return listWatchPointsHelp;
	} else if (tokens[1] == "set_condition") {
		return setCondHelp;
	} else if (tokens[1] == "remove_condition") {
		return removeCondHelp;
	} else if (tokens[1] == "list_conditions") {
		return listCondHelp;
	} else if (tokens[1] == "probe") {
		return probeHelp;
	} else if (tokens[1] == "cont") {
		return contHelp;
	} else if (tokens[1] == "step") {
		return stepHelp;
	} else if (tokens[1] == "break") {
		return breakHelp;
	} else if (tokens[1] == "breaked") {
		return breakedHelp;
	} else if (tokens[1] == "disasm") {
		return disasmHelp;
	} else {
		return unknownHelp;
	}
}

set<string> DebugCmd::getBreakPointIdsAsStringSet() const
{
	MSXCPUInterface& interface = debugger.motherBoard.getCPUInterface();
	const BreakPoints& breakPoints = interface.getBreakPoints();
	set<string> bpids;
	for (BreakPoints::const_iterator it = breakPoints.begin();
	     it != breakPoints.end(); ++it) {
		bpids.insert(StringOp::Builder() << "bp#" << it->second->getId());
	}
	return bpids;
}
set<string> DebugCmd::getWatchPointIdsAsStringSet() const
{
	MSXCPUInterface& interface = debugger.motherBoard.getCPUInterface();
	const MSXCPUInterface::WatchPoints& watchPoints = interface.getWatchPoints();
	set<string> wpids;
	for (MSXCPUInterface::WatchPoints::const_iterator it = watchPoints.begin();
	     it != watchPoints.end(); ++it) {
		wpids.insert(StringOp::Builder() << "wp#" << (*it)->getId());
	}
	return wpids;
}
set<string> DebugCmd::getConditionIdsAsStringSet() const
{
	MSXCPUInterface& interface = debugger.motherBoard.getCPUInterface();
	const MSXCPUInterface::Conditions& conditions = interface.getConditions();
	set<string> condids;
	for (MSXCPUInterface::Conditions::const_iterator it = conditions.begin();
	     it != conditions.end(); ++it) {
		condids.insert(StringOp::Builder() << "cond#" << (*it)->getId());
	}
	return condids;
}

void DebugCmd::tabCompletion(vector<string>& tokens) const
{
	set<string> singleArgCmds;
	singleArgCmds.insert("list");
	singleArgCmds.insert("step");
	singleArgCmds.insert("cont");
	singleArgCmds.insert("break");
	singleArgCmds.insert("breaked");
	singleArgCmds.insert("list_bp");
	singleArgCmds.insert("list_watchpoints");
	singleArgCmds.insert("list_conditions");
	set<string> debuggableArgCmds;
	debuggableArgCmds.insert("desc");
	debuggableArgCmds.insert("size");
	debuggableArgCmds.insert("read");
	debuggableArgCmds.insert("read_block");
	debuggableArgCmds.insert("write");
	debuggableArgCmds.insert("write_block");
	set<string> otherCmds;
	otherCmds.insert("disasm");
	otherCmds.insert("set_bp");
	otherCmds.insert("remove_bp");
	otherCmds.insert("set_watchpoint");
	otherCmds.insert("remove_watchpoint");
	otherCmds.insert("set_condition");
	otherCmds.insert("remove_condition");
	otherCmds.insert("probe");
	switch (tokens.size()) {
	case 2: {
		set<string> cmds;
		cmds.insert(singleArgCmds.begin(), singleArgCmds.end());
		cmds.insert(debuggableArgCmds.begin(), debuggableArgCmds.end());
		cmds.insert(otherCmds.begin(), otherCmds.end());
		completeString(tokens, cmds);
		break;
	}
	case 3:
		if (singleArgCmds.find(tokens[1]) ==
				singleArgCmds.end()) {
			// this command takes (an) argument(s)
			if (debuggableArgCmds.find(tokens[1]) !=
				debuggableArgCmds.end()) {
				// it takes a debuggable here
				set<string> debuggables;
				debugger.getDebuggables(debuggables);
				completeString(tokens, debuggables);
			} else if (tokens[1] == "remove_bp") {
				// this one takes a bp id
				set<string> bpids = getBreakPointIdsAsStringSet();
				completeString(tokens, bpids);
			} else if (tokens[1] == "remove_watchpoint") {
				// this one takes a wp id
				set<string> wpids = getWatchPointIdsAsStringSet();
				completeString(tokens, wpids);
			} else if (tokens[1] == "remove_condition") {
				// this one takes a cond id
				set<string> condids = getConditionIdsAsStringSet();
				completeString(tokens, condids);
			} else if (tokens[1] == "set_watchpoint") {
				set<string> types;
				types.insert("write_io");
				types.insert("write_mem");
				types.insert("read_io");
				types.insert("read_mem");
				completeString(tokens, types);
			} else if (tokens[1] == "probe") {
				set<string> subCmds;
				subCmds.insert("list");
				subCmds.insert("desc");
				subCmds.insert("read");
				subCmds.insert("set_bp");
				subCmds.insert("remove_bp");
				subCmds.insert("list_bp");
				completeString(tokens, subCmds);
			}
		}
		break;
	case 4:
		if ((tokens[1] == "probe") &&
		    ((tokens[2] == "desc") || (tokens[2] == "read") ||
		     (tokens[2] == "set_bp"))) {
			set<string> probes;
			debugger.getProbes(probes);
			completeString(tokens, probes);
		}
		break;
	}
}

} // namespace openmsx
