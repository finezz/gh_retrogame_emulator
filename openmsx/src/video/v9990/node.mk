# $Id: node.mk 12659 2012-06-23 19:59:14Z m9710797 $

include build/node-start.mk

SRC_HDR:= \
	V9990 V9990VRAM \
	V9990CmdEngine \
	V9990DisplayTiming \
	V9990Renderer \
	V9990DummyRenderer \
	V9990PixelRenderer \
	V9990SDLRasterizer \
	V9990BitmapConverter \
	V9990P1Converter \
	V9990P2Converter \
	Video9000 \

HDR_ONLY:= \
	V9990Rasterizer \
	V9990Renderer \
	V9990ModeEnum \

include build/node-end.mk

