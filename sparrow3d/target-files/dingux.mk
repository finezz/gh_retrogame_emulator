# === Target file for the Dingoo with Dingux. Use it with "make TARGET=dingux" ===

CPP = mipsel-linux-gcc -DMOBILE_DEVICE -DDINGUX -DFAST_BUT_UGLY $(GENERAL_TWEAKS) $(SMALL_RESOLUTION_DEVICES)
SDL = -I/home/steward/Downloads/buildroot-2017.02.9/output/host/usr/mipsel-buildroot-linux-uclibc/sysroot/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT
INCLUDE = 
LIB = 
