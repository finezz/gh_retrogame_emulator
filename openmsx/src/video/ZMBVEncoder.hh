// $Id: ZMBVEncoder.hh 11636 2010-08-12 21:03:37Z m9710797 $

// Code based on DOSBox-0.65

#ifndef ZMBVENCODER_HH
#define ZMBVENCODER_HH

#include "MemBuffer.hh"
#include <zlib.h>

struct SDL_PixelFormat;

namespace openmsx {

class FrameSource;
template<class P> class PixelOperations;

class ZMBVEncoder
{
public:
	static const char* CODEC_4CC;

	ZMBVEncoder(unsigned width, unsigned height, unsigned bpp);
	~ZMBVEncoder();

	void compressFrame(bool keyFrame, FrameSource* frame,
	                   void*& buffer, unsigned& written);

private:
	enum Format {
		ZMBV_FORMAT_16BPP = 6,
		ZMBV_FORMAT_32BPP = 8
	};

	void setupBuffers(unsigned bpp);
	unsigned neededSize();
	template<class P> void addFullFrame(const SDL_PixelFormat& pixelFormat);
	template<class P> void addXorFrame(const SDL_PixelFormat& pixelFormat);
	template<class P> unsigned possibleBlock(int vx, int vy, unsigned offset);
	template<class P> unsigned compareBlock(int vx, int vy, unsigned offset);
	template<class P> void addXorBlock(
		const PixelOperations<P>& pixelOps, int vx, int vy, unsigned offset);
	const void* getScaledLine(FrameSource* frame, unsigned y);

	MemBuffer<unsigned char> oldframe;
	MemBuffer<unsigned char> newframe;
	MemBuffer<unsigned char> work;
	MemBuffer<unsigned char> output;
	MemBuffer<unsigned> blockOffsets;
	unsigned outputSize;
	unsigned workUsed;

	z_stream zstream;

	const unsigned width;
	const unsigned height;
	unsigned pitch;
	unsigned pixelSize;
	Format format;
};

} // namespace openmsx

#endif
