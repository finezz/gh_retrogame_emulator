.PHONY: all clean

CC:=mipsel-linux-gcc

SYSROOT:=$(shell $(CC) -print-sysroot)
CFLAGS+=$(shell $(SYSROOT)/usr/bin/sdl-config --cflags)
LDFLAGS+=$(shell $(SYSROOT)/usr/bin/sdl-config --libs)

CFLAGS+=-std=c99 -W -Wall -Wextra -O2

SOURCES:=$(wildcard *.c)
BINARIES:=$(foreach source,$(SOURCES),$(source:%.c=%))

all: $(BINARIES)

clean:
	rm -f $(BINARIES)

$(BINARIES): %: %.c
