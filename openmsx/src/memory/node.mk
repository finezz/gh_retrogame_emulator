# $Id: node.mk 12545 2012-05-22 17:25:13Z m9710797 $

include build/node-start.mk

SRC_HDR:= \
	MSXMapperIO \
	MSXMemoryMapper \
	MSXRam \
	MSXMirrorDevice \
	MSXRomCLI \
	Ram SRAM CheckedRam \
	Rom AmdFlash \
	RomInfo RomFactory RomDatabase \
	RomInfoTopic \
	MSXRom \
	RomBlocks \
	RomPlain RomPageNN RomGeneric8kB RomGeneric16kB \
	RomDRAM RomKonami RomKonamiSCC RomKonamiKeyboardMaster \
	RomAscii8kB RomAscii8_8 RomAscii16kB \
	RomPadial8kB RomPadial16kB RomMSXDOS2 \
	RomAscii16_2 RomRType RomCrossBlaim RomHarryFox \
	RomGameMaster2 RomMajutsushi RomSynthesizer \
	RomZemina80in1 RomZemina90in1 RomZemina126in1 \
	RomPanasonic RomNational RomSuperLodeRunner \
	RomHalnote RomHolyQuran RomHolyQuran2 \
	RomFSA1FM RomPlayBall RomNettouYakyuu \
	PanasonicMemory PanasonicRam \
	MSXMegaRam \
	MSXPac \
	MSXHBI55 \
	ESE_RAM ESE_SCC \
	RomManbow2 RomMatraInk RomArc RomDooly \
	MegaFlashRomSCCPlus

HDR_ONLY:= \
	RomTypes \
	RomBlockDebuggable \

include build/node-end.mk

