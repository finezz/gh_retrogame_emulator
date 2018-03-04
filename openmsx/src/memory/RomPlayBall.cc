// $Id: RomPlayBall.cc 12527 2012-05-17 17:34:11Z m9710797 $

#include "RomPlayBall.hh"
#include "SamplePlayer.hh"
#include "CacheLine.hh"
#include "Rom.hh"
#include "FileOperations.hh"
#include "serialize.hh"

namespace openmsx {

RomPlayBall::RomPlayBall(const DeviceConfig& config, std::auto_ptr<Rom> rom_)
	: Rom16kBBlocks(config, rom_)
	, samplePlayer(new SamplePlayer(
		"Playball-DAC", "Sony Playball's DAC", config,
		FileOperations::stripExtension(rom->getFilename()) + '_',
		15, "playball/playball_"))
{
	setUnmapped(0);
	setRom(1, 0);
	setRom(2, 1);
	setUnmapped(3);

	reset(EmuTime::dummy());
}

RomPlayBall::~RomPlayBall()
{
}

void RomPlayBall::reset(EmuTime::param /*time*/)
{
	samplePlayer->reset();
}

byte RomPlayBall::peekMem(word address, EmuTime::param time) const
{
	if (address == 0xBFFF) {
		return samplePlayer->isPlaying() ? 0xFE : 0xFF;
	} else {
		return Rom16kBBlocks::peekMem(address, time);
	}
}

byte RomPlayBall::readMem(word address, EmuTime::param time)
{
	return RomPlayBall::peekMem(address, time);
}

const byte* RomPlayBall::getReadCacheLine(word address) const
{
	if ((address & CacheLine::HIGH) == (0xBFFF & CacheLine::HIGH)) {
		return NULL;
	} else {
		return Rom16kBBlocks::getReadCacheLine(address);
	}
}

void RomPlayBall::writeMem(word address, byte value, EmuTime::param /*time*/)
{
	if (address == 0xBFFF) {
		if ((value <= 14) && !samplePlayer->isPlaying()) {
			samplePlayer->play(value);
		}
	}
}

byte* RomPlayBall::getWriteCacheLine(word address) const
{
	if ((address & CacheLine::HIGH) == (0xBFFF & CacheLine::HIGH)) {
		return NULL;
	} else {
		return unmappedWrite;
	}
}

template<typename Archive>
void RomPlayBall::serialize(Archive& ar, unsigned /*version*/)
{
	ar.template serializeBase<Rom16kBBlocks>(*this);
	ar.serialize("SamplePlayer", *samplePlayer);
}
INSTANTIATE_SERIALIZE_METHODS(RomPlayBall);
REGISTER_MSXDEVICE(RomPlayBall, "RomPlayBall");

} // namespace openmsx
