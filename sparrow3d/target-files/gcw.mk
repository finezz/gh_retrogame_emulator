# === Target file for the GCW. Use it with "make TARGET=gcw" ===

CPP = mipsel-linux-gcc -DMOBILE_DEVICE -DGCW -DFAST_BUT_UGLY $(GENERAL_TWEAKS) $(SMALL_RESOLUTION_DEVICES)
SDL = -I/home/steward/Downloads/buildroot-2017.02.9/output/host/usr/mipsel-buildroot-linux-uclibc/sysroot/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT
INCLUDE = 
LIB = 
