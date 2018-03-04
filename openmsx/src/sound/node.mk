# $Id: node.mk 12645 2012-06-19 17:40:36Z m9710797 $

include build/node-start.mk

SRC_HDR:= \
	Mixer MSXMixer NullSoundDriver \
	SDLSoundDriver DirectXSoundDriver \
	SoundDevice ResampledSoundDevice \
	ResampleTrivial ResampleHQ ResampleLQ ResampleBlip \
	BlipBuffer \
	MSXPSG AY8910 AY8910Periphery \
	DACSound16S DACSound8U \
	KeyClick \
	SCC MSXSCCPlusCart \
	VLM5030 \
	MSXAudio \
	EmuTimer \
	Y8950 Y8950Adpcm Y8950KeyboardConnector \
	Y8950Periphery \
	Y8950KeyboardDevice DummyY8950KeyboardDevice \
	MSXFmPac MSXMusic \
	YM2413 YM2413Okazaki YM2413Burczynski \
	MSXTurboRPCM \
	YMF262 YMF278 MSXMoonSound MSXOPL3Cartridge \
	YM2151 MSXYamahaSFG \
	AudioInputConnector AudioInputDevice \
	DummyAudioInputDevice WavAudioInput \
	WavWriter \
	SamplePlayer \
	WavData

HDR_ONLY:= \
	SoundDriver \
	ResampleAlgo \
	BlipConfig \
	YM2413Core \
	YM2413OkazakiConfig \
	DummyAY8910Periphery

SRC_HDR_$(COMPONENT_AO)+= \
	LibAOSoundDriver

DIST:= \
	ResampleCoeffs.ii \
	BlipTable.ii \
	YM2413OkazakiTable.ii \
	ResampleHQ-x64.asm \
	ResampleHQ-x86.asm \
	generateBlipTable.cc \
	generateYM2413OkazakiTable.cc \

#TODO
#TEST:= YM2413Test

include build/node-end.mk

