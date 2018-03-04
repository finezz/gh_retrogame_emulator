// $Id: PreCacheFile.cc 11574 2010-07-07 08:08:47Z m9710797 $

#include "PreCacheFile.hh"
#include "FileOperations.hh"
#include "statp.hh"
#include <cstdio>
#include <sys/types.h>

namespace openmsx {

PreCacheFile::PreCacheFile(const std::string& name_)
	: name(name_), thread(this), sem(1), exitLoop(false)
{
	thread.start();
}

PreCacheFile::~PreCacheFile()
{
	{
		ScopedLock lock(sem);
		exitLoop = true;
	}
	thread.join();
}

void PreCacheFile::run()
{
	struct stat st;
	if (stat(name.c_str(), &st)) return;
	if (!S_ISREG(st.st_mode)) {
		// don't pre-cache non regular files (e.g. /dev/fd0)
		return;
	}

	FILE* file = FileOperations::openFile(name, "rb");
	if (!file) return;

	fseek(file, 0, SEEK_END);
	unsigned size = unsigned(ftell(file));
	if (size < 1024 * 1024) {
		// only pre-cache small files

		const unsigned BLOCK_SIZE = 4096;
		unsigned block = 0;
		unsigned repeat = 0;
		while (true) {
			bool exitLoop2;
			{
				ScopedLock lock(sem);
				exitLoop2 = exitLoop;
			}
			if (exitLoop2) break;

			char buf[BLOCK_SIZE];
			fseek(file, block * BLOCK_SIZE, SEEK_SET);
			size_t read = fread(buf, 1, BLOCK_SIZE, file);
			if (read != BLOCK_SIZE) {
				// error or end-of-file reached,
				// in both cases stop pre-caching
				break;
			}

			// Just reading a file linearly from front to back
			// makes Linux classify the read as a 'streaming read'.
			// Linux doesn't cache those. To avoid this we read
			// some of the blocks twice.
			if (repeat != 0) {
				--repeat; ++block;
			} else {
				repeat = 5;
			}
		}
	}
	fclose(file);
}

} // namespace openmsx
