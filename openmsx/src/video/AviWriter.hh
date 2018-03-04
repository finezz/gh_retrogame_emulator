// $Id: AviWriter.hh 12823 2012-08-19 19:01:36Z m9710797 $

// Code based on DOSBox-0.65

#ifndef AVIWRITER_HH
#define AVIWRITER_HH

#include "endian.hh"
#include <vector>
#include <memory>

namespace openmsx {

class File;
class Filename;
class FrameSource;
class ZMBVEncoder;

class AviWriter
{
public:
	AviWriter(const Filename& filename, unsigned width, unsigned height,
	          unsigned bpp, unsigned channels, unsigned freq);
	~AviWriter();
	void addFrame(FrameSource* frame, unsigned samples, short* sampleData);
	void setFps(double fps);

private:
	void addAviChunk(const char* tag, unsigned size, void* data, unsigned flags);

	std::auto_ptr<File> file;
	const std::auto_ptr<ZMBVEncoder> codec;
	std::vector<Endian::L32> index;

	double fps;
	const unsigned width;
	const unsigned height;
	const unsigned channels;
	const unsigned audiorate;

	unsigned frames;
	unsigned audiowritten;
	unsigned written;
};

} // namespace openmsx

#endif
