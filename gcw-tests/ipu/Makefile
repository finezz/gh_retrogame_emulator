TARGET := ipu
CFLAGS := -Wall -O2 -lSDL_image -lSDL -I/home/steward/Github/gh_retrogame_toolchain/mipsel-linux-gcc/usr/mipsel-buildroot-linux-uclibc/sysroot/usr/include/SDL
LDFLAGS := -lm

CROSS_COMPILE ?= mipsel-linux-
CC := $(CROSS_COMPILE)gcc
OBJS := ipu.o

.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJS)
