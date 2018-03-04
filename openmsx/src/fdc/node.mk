# $Id: node.mk 12729 2012-07-14 17:41:22Z manuelbi $

include build/node-start.mk

SRC_HDR:= \
	MSXFDC \
	WD2793BasedFDC \
	PhilipsFDC \
	NationalFDC \
	VictorFDC \
	SanyoFDC \
	MicrosolFDC \
	AVTFDC \
	WD2793 \
	TurboRFDC \
	TC8566AF \
	DiskImageCLI \
	DiskDrive \
	RealDrive \
	DiskChanger \
	DiskFactory \
	DriveMultiplexer \
	Disk \
	RawTrack \
	DiskName \
	SectorBasedDisk \
	DummyDisk \
	DSKDiskImage \
	XSADiskImage \
	DMKDiskImage \
	DirAsDSK \
	EmptyDiskPatch \
	RamDSKDiskImage \
	DiskPartition \
	MSXtar \
	DiskImageUtils \
	DiskContainer \
	SectorAccessibleDisk \
	DiskManipulator \
	BootBlocks \
	NowindInterface NowindHost NowindRomDisk NowindCommand

HDR_ONLY:= \
	DiskExceptions \

include build/node-end.mk

