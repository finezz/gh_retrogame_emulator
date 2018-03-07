##############################################################################
# Non-user-configurable settings
##############################################################################

# *** Comment out this line to get verbose make output, for debugging build
# problems
QUIET = 1


##############################################################################
# CPU-dependent settings
##############################################################################
#note : -D__CPU_$(MY_CPU) is added automatically later on.
CFLAGS.i386       = -DLSB_FIRST -DX86_ASM
CFLAGS.i386_noasm = -DLSB_FIRST
CFLAGS.ia64       = -DLSB_FIRST -DALIGN_INTS -DALIGN_SHORTS -D__LP64__
CFLAGS.amd64      = -DLSB_FIRST -DALIGN_INTS -DALIGN_SHORTS -D__LP64__
CFLAGS.alpha      = -DLSB_FIRST -DALIGN_INTS -DALIGN_SHORTS -D__LP64__
CFLAGS.m68k       = 
CFLAGS.risc       = -DALIGN_INTS -DALIGN_SHORTS 
CFLAGS.risc_lsb   = -DALIGN_INTS -DALIGN_SHORTS -DLSB_FIRST
CFLAGS.mips       = -DALIGN_INTS -DALIGN_SHORTS -DSGI_FIX_MWA_NOP

##############################################################################
# Architecture-dependent settings
##############################################################################
LIBS.solaris       = -lnsl -lsocket
LIBS.irix          = -laudio
LIBS.irix_al       = -laudio
LIBS.aix           = -lUMSobj
LIBS.next	   = -framework SoundKit
LIBS.macosx	   = -framework CoreAudio
#LIBS.openbsd       = -lossaudio
LIBS.nto	   = -lsocket -lasound
LIBS.beos          = `$(SDL_CONFIG) --libs`
LIBS.linux         = 

ifeq ($(PROF),GEN)
    LIBS.linux += -lgcov
endif


##############################################################################
# Display-dependent settings
##############################################################################
#first calculate the X11 Joystick driver settings, this is done here since
#they are only valid for X11 based display methods
ifdef JOY_X11
JOY_X11_CFLAGS = -DX11_JOYSTICK "-DX11_JOYNAME='$(X11_JOYNAME)'" -DUSE_X11_JOYEVENTS
JOY_X11_LIBS   = -lXi
endif

ifdef XINPUT_DEVICES
XINPUT_DEVICES_CFLAGS = -DUSE_XINPUT_DEVICES
XINPUT_DEVICES_LIBS = -lXi
endif

# svga and ggi also use $(X11LIB) since that's where zlib often is
LIBS.x11        = $(X11LIB) $(JOY_X11_LIBS) $(XINPUT_DEVICES_LIBS) -lX11 -lXext
LIBS.svgalib    = $(X11LIB) -lvga -lvgagl
LIBS.ggi        = $(X11LIB) -lggi
LIBS.xgl        = $(X11LIB) $(JOY_X11_LIBS) -lX11 -lXext $(GLLIBS) -ljpeg
LIBS.xfx        = $(X11LIB) $(JOY_X11_LIBS) -lX11 -lXext -lglide2x
LIBS.svgafx     = $(X11LIB) -lvga -lvgagl -lglide2x
LIBS.openstep	= -framework AppKit
#LIBS.SDL	= $(X11LIB) `$(SDL_CONFIG) --static-libs`
LIBS.SDL	= $(X11LIB) `$(SDL_CONFIG) --libs`
LIBS.photon2	= -L/usr/lib -lph -lphrender

CFLAGS.x11      = $(X11INC) $(JOY_X11_CFLAGS) $(XINPUT_DEVICES_CFLAGS)
CFLAGS.xgl      = $(X11INC) $(JOY_X11_CFLAGS) $(GLCFLAGS)
CFLAGS.xfx      = $(X11INC) $(JOY_X11_CFLAGS) -I/usr/include/glide
CFLAGS.svgafx   = -I/usr/include/glide
CFLAGS.SDL      = $(X11INC) `$(SDL_CONFIG) --cflags` -D_REENTRANT
CFLAGS.photon2	=

ifdef X11_DGA
INST.x11        = doinstallsuid
else
INST.x11	= doinstall
endif
INST.ggi        = doinstall
INST.svgalib    = doinstallsuid
INST.xgl        = doinstallsuid copycab
INST.xfx        = doinstallsuid
INST.svgafx     = doinstallsuid
INST.SDL	= doinstall
INST.photon2	= doinstall

# handle X11 display method additonal settings
ifdef X11_MITSHM
CFLAGS.x11 += -DUSE_MITSHM
endif
ifdef X11_XV
CFLAGS.x11 += -DUSE_XV -DUSE_HWSCALE
LIBS.x11   += -lXv
endif
ifdef X11_DGA
CFLAGS.x11 += -DUSE_DGA
LIBS.x11   += -lXxf86dga -lXxf86vm
endif
ifdef TDFX_DGA_WORKAROUND
CFLAGS.x11 +=  -DTDFX_DGA_WORKAROUND 
endif
ifdef X11_XIL
CFLAGS.x11 += -DUSE_XIL
LIBS.x11   += -lxil -lpthread
endif


##############################################################################
# Quiet the compiler output if requested
##############################################################################

ifdef QUIET
CC_COMMENT = 
CC_COMPILE = @
AR_OPTS = rc
else
CC_COMMENT = \#
CC_COMPILE = 
AR_OPTS = rcv
endif


##############################################################################
# these are the object subdirectories that need to be created.
##############################################################################
OBJ     = $(NAME).obj

OBJDIR = $(OBJ)/unix.$(DISPLAY_METHOD)

OBJDIRS = $(OBJ) $(OBJ)/cpu $(OBJ)/sound $(OBJ)/drivers \
	  $(OBJ)/machine $(OBJ)/vidhrdw $(OBJ)/sndhrdw
ifeq ($(TARGET), mess)
OBJDIRS += $(OBJ)/mess $(OBJ)/mess/expat $(OBJ)/mess/cpu \
	   $(OBJ)/mess/devices $(OBJ)/mess/systems $(OBJ)/mess/machine \
	   $(OBJ)/mess/vidhrdw $(OBJ)/mess/sndhrdw $(OBJ)/mess/formats \
	   $(OBJ)/mess/tools $(OBJ)/mess/tools/dat2html \
	   $(OBJ)/mess/tools/mkhdimg $(OBJ)/mess/tools/messroms \
	   $(OBJ)/mess/tools/imgtool $(OBJ)/mess/tools/messdocs \
	   $(OBJ)/mess/tools/messtest $(OBJ)/mess/tools/mkimage \
	   $(OBJ)/mess/sound
endif

UNIX_OBJDIR = $(OBJ)/unix.$(DISPLAY_METHOD)

SYSDEP_DIR = $(UNIX_OBJDIR)/sysdep
DSP_DIR = $(UNIX_OBJDIR)/sysdep/dsp-drivers
MIXER_DIR = $(UNIX_OBJDIR)/sysdep/mixer-drivers
VID_DIR = $(UNIX_OBJDIR)/video-drivers
JOY_DIR = $(UNIX_OBJDIR)/joystick-drivers
FRAMESKIP_DIR = $(UNIX_OBJDIR)/frameskip-drivers

OBJDIRS += $(UNIX_OBJDIR) $(SYSDEP_DIR) $(DSP_DIR) $(MIXER_DIR) $(VID_DIR) \
	$(JOY_DIR) $(FRAMESKIP_DIR)

IMGTOOL_LIBS = -lz

ifeq ($(TARGET), mess)
INCLUDE_PATH = -I. -Imess -Isrc -Isrc/includes -Isrc/unix -Isrc/unix/sysdep -I$(OBJ)/cpu/m68000 -Isrc/cpu/m68000
else
INCLUDE_PATH = -I. -Isrc -Isrc/includes -Isrc/unix -I$(OBJ)/cpu/m68000 -Isrc/cpu/m68000
endif

##############################################################################
# "Calculate" the final CFLAGS, unix CONFIG, LIBS and OBJS
##############################################################################
ifdef ZLIB
ZLIB = src/unix/contrib/cutzlib-1.2.1/libz.a
endif

all: maketree $(NAME).$(DISPLAY_METHOD) tools

# CPU core include paths
VPATH = src $(wildcard src/cpu/*)

# Platform-dependent objects for imgtool
PLATFORM_IMGTOOL_OBJS = $(OBJDIR)/dirio.o \
			$(OBJDIR)/fileio.o \
			$(OBJDIR)/sysdep/misc.o

include src/core.mak

ifeq ($(TARGET), mess)
include mess/mess.mak
else
include src/$(TARGET).mak
endif

include src/rules.mak

ifeq ($(TARGET), mess)
include mess/rules_ms.mak
endif

ifdef DEBUG
DBGDEFS = -DMAME_DEBUG
else
DBGDEFS =
DBGOBJS =
endif

# Perhaps one day original mame/mess sources will use POSIX strcasecmp and
# M_PI instead MS-DOS counterparts... ( a long and sad history ...)
MY_CFLAGS = $(CFLAGS) $(IL) $(CFLAGS.$(MY_CPU)) \
	-D__ARCH_$(ARCH) -D__CPU_$(MY_CPU) -D$(DISPLAY_METHOD) \
	-Dstricmp=strcasecmp -Dstrnicmp=strncasecmp \
	-DPI=M_PI -DXMAME -DUNIX -DSIGNED_SAMPLES -DCLIB_DECL= \
	$(COREDEFS) $(SOUNDDEFS) $(CPUDEFS) $(ASMDEFS) \
	$(INCLUDES) $(INCLUDE_PATH)

MY_LIBS = $(LIBS) $(LIBS.$(ARCH)) $(LIBS.$(DISPLAY_METHOD)) -lz

ifdef SEPARATE_LIBM
MY_LIBS += -lm -lc
endif

ifdef ZLIB
MY_CFLAGS += -Isrc/unix/contrib/cutzlib-1.2.1 -I../../contrib/cutzlib-1.2.1
LDFLAGS   += -Lsrc/unix/contrib/cutzlib-1.2.1
endif

ifdef DEBUG
MY_CFLAGS += -DMAME_DEBUG
MY_LIBS   += -lcurses
endif

ifdef LOW_MEM
MY_CFLAGS += -DLOW_MEM
endif

ifdef XMAME_NET
MY_CFLAGS += -DXMAME_NET
endif

ifdef HAVE_MEMMOVE
MY_CFLAGS += -DHAVE_MEMMOVE
endif

ifdef HAVE_BCOPY
MY_CFLAGS += -DHAVE_BCOPY
endif

ifdef HAVE_MPROTECT
MY_CFLAGS += -DHAVE_MPROTECT
endif

##############################################################################
# Object listings
##############################################################################

# common objs
COMMON_OBJS  =  \
	$(OBJDIR)/main.o $(OBJDIR)/sound.o \
	$(OBJDIR)/keyboard.o $(OBJDIR)/devices.o \
	$(OBJDIR)/video.o $(OBJDIR)/mode.o \
	$(OBJDIR)/fileio.o $(OBJDIR)/dirio.o $(OBJDIR)/config.o \
	$(OBJDIR)/fronthlp.o $(OBJDIR)/ident.o $(OBJDIR)/network.o \
	$(OBJDIR)/snprintf.o $(OBJDIR)/nec765_dummy.o \
	$(OBJDIR)/ticker.o $(OBJDIR)/parallel.o $(OBJDIR)/fileio_more.o

#$(OBJDIR)/effect.o \

# sysdep objs
SYSDEP_OBJS = $(SYSDEP_DIR)/rc.o $(SYSDEP_DIR)/misc.o \
   $(SYSDEP_DIR)/plugin_manager.o $(SYSDEP_DIR)/sound_stream.o \
   $(SYSDEP_DIR)/sysdep_palette.o $(SYSDEP_DIR)/sysdep_dsp.o \
   $(SYSDEP_DIR)/sysdep_mixer.o

# video driver objs per display method
VID_OBJS.x11    = $(VID_DIR)/xinput.o $(VID_DIR)/xil.o \
	$(VID_DIR)/x11_window.o \
	$(VID_DIR)/xf86_dga1.o $(VID_DIR)/xf86_dga2.o $(VID_DIR)/xf86_dga.o
VID_OBJS.xgl    = $(VID_DIR)/gltool.o $(VID_DIR)/glxtool.o $(VID_DIR)/glcaps.o \
		  $(VID_DIR)/glvec.o $(VID_DIR)/glgen.o $(VID_DIR)/glexport.o \
		  $(VID_DIR)/glcab.o $(VID_DIR)/gljpg.o \
		  $(VID_DIR)/xinput.o
VID_OBJS.xfx    = $(VID_DIR)/fxgen.o $(VID_DIR)/xinput.o
VID_OBJS.svgalib = $(VID_DIR)/svgainput.o
VID_OBJS.svgafx = $(VID_DIR)/svgainput.o $(VID_DIR)/fxgen.o
VID_OBJS.openstep = $(VID_DIR)/openstep_input.o
VID_OBJS.photon2 = $(VID_DIR)/photon2_input.o \
	$(VID_DIR)/photon2_window.o \
	$(VID_DIR)/photon2_overlay.o
VID_OBJS = $(VID_DIR)/$(DISPLAY_METHOD).o $(VID_OBJS.$(DISPLAY_METHOD))

# sound driver objs per arch
#SOUND_OBJS.linux   = $(DSP_DIR)/oss.o $(MIXER_DIR)/oss.o $(DSP_DIR)/alsa.o
SOUND_OBJS.linux   = 
SOUND_OBJS.freebsd = $(DSP_DIR)/oss.o $(MIXER_DIR)/oss.o
SOUND_OBJS.netbsd  = $(DSP_DIR)/netbsd.o
#SOUND_OBJS.openbsd = $(DSP_DIR)/oss.o $(MIXER_DIR)/oss.o
SOUND_OBJS.openbsd = $(DSP_DIR)/netbsd.o 
SOUND_OBJS.solaris = $(DSP_DIR)/solaris.o $(MIXER_DIR)/solaris.o
SOUND_OBJS.next    = $(DSP_DIR)/soundkit.o
SOUND_OBJS.macosx  = $(DSP_DIR)/coreaudio.o
SOUND_OBJS.nto     = $(DSP_DIR)/io-audio.o
SOUND_OBJS.irix    = $(DSP_DIR)/irix.o
SOUND_OBJS.irix_al = $(DSP_DIR)/irix_al.o
SOUND_OBJS.beos    =
SOUND_OBJS.generic =
#these need to be converted to plugins first
#SOUND_OBJS.aix     = $(DSP_DIR)/aix.o
SOUND_OBJS = $(SOUND_OBJS.$(ARCH)) $(DSP_DIR)/esound.o $(DSP_DIR)/alsa.o $(DSP_DIR)/artssound.o \
    $(DSP_DIR)/arts.o $(DSP_DIR)/sdl.o $(DSP_DIR)/waveout.o

# joystick objs
JOY_OBJS = $(JOY_DIR)/joy_i386.o $(JOY_DIR)/joy_pad.o $(JOY_DIR)/joy_x11.o \
    $(JOY_DIR)/joy_usb.o $(JOY_DIR)/joy_ps2.o $(JOY_DIR)/joy_SDL.o \
    $(JOY_DIR)/XInputDevices.o $(JOY_DIR)/lightgun_abs_event.o

# framskip objs
FRAMESKIP_OBJS = $(FRAMESKIP_DIR)/dos.o $(FRAMESKIP_DIR)/barath.o

# all objs
UNIX_OBJS = $(COMMON_OBJS) $(SYSDEP_OBJS) $(VID_OBJS) $(SOUND_OBJS) $(JOY_OBJS) \
   $(FRAMESKIP_OBJS)

##############################################################################
# CFLAGS
##############################################################################

# per arch
#CFLAGS.linux      = -DSYSDEP_DSP_OSS -DSYSDEP_MIXER_OSS -DHAVE_SNPRINTF -DHAVE_VSNPRINTF
CFLAGS.linux      = -DHAVE_SNPRINTF -DHAVE_VSNPRINTF
CFLAGS.freebsd    = -DSYSDEP_DSP_OSS -DSYSDEP_MIXER_OSS -DHAVE_SNPRINTF -DHAVE_VSNPRINTF
CFLAGS.netbsd     = -DSYSDEP_DSP_NETBSD -DHAVE_SNPRINTF -DHAVE_VSNPRINTF
#CFLAGS.openbsd    = -DSYSDEP_DSP_OSS -DSYSDEP_MIXER_OSS -DHAVE_SNPRINTF -DHAVE_VSNPRINTF
CFLAGS.openbsd    = -DSYSDEP_DSP_NETBSD -DHAVE_SNPRINTF -DHAVE_VSNPRINTF
CFLAGS.solaris    = -DSYSDEP_DSP_SOLARIS -DSYSDEP_MIXER_SOLARIS
CFLAGS.next       = -DSYSDEP_DSP_SOUNDKIT -DBSD43
CFLAGS.macosx     = -DSYSDEP_DSP_COREAUDIO -DHAVE_SNPRINTF -DHAVE_VSNPRINTF
CFLAGS.nto        = -DSYSDEP_DSP_ALSA -DSYSDEP_MIXER_ALSA
CFLAGS.irix       = -DSYSDEP_DSP_IRIX -DHAVE_SNPRINTF
CFLAGS.irix_al    = -DSYSDEP_DSP_IRIX -DHAVE_SNPRINTF
CFLAGS.beos       = `sdl-config --cflags` -DSYSDEP_DSP_SDL
CFLAGS.generic    =
#these need to be converted to plugins first
#CFLAGS.aix        = -DSYSDEP_DSP_AIX -I/usr/include/UMS -I/usr/lpp/som/include

# CONFIG are the cflags used to build the unix tree, this is where most defines
# go
CONFIG = $(MY_CFLAGS) $(CFLAGS.$(DISPLAY_METHOD)) -DNAME='"x$(TARGET)"' \
	-DDISPLAY_METHOD='"$(DISPLAY_METHOD)"' \
	-DXMAMEROOT='"$(XMAMEROOT)"' -DSYSCONFDIR='"$(SYSCONFDIR)"' \
	$(CFLAGS.$(ARCH))

ifdef HAVE_GETTIMEOFDAY
CONFIG += -DHAVE_GETTIMEOFDAY
endif

# Sound drivers config
ifdef SOUND_ESOUND
CONFIG  += -DSYSDEP_DSP_ESOUND `esd-config --cflags`
MY_LIBS += `esd-config --libs`
endif

ifdef SOUND_ALSA
CONFIG  += -DSYSDEP_DSP_ALSA 
MY_LIBS += -lasound
endif

ifdef SOUND_ARTS_TEIRA
CONFIG  += -DSYSDEP_DSP_ARTS_TEIRA `artsc-config --cflags`
MY_LIBS += `artsc-config --libs`
endif

ifdef SOUND_ARTS_SMOTEK
CONFIG  += -DSYSDEP_DSP_ARTS_SMOTEK `artsc-config --cflags`
MY_LIBS += `artsc-config --libs`
endif

ifdef SOUND_SDL
CONFIG  += -DSYSDEP_DSP_SDL `$(SDL_CONFIG) --cflags`
#MY_LIBS += `$(SDL_CONFIG) --libs`
endif

ifdef SOUND_WAVEOUT
CONFIG  += -DSYSDEP_DSP_WAVEOUT
endif

# Joystick drivers config
ifdef JOY_I386
CONFIG += -DI386_JOYSTICK
endif
ifdef JOY_PAD
CONFIG += -DLIN_FM_TOWNS
endif
ifdef JOY_PS2
CONFIG += -DPS2_JOYSTICK
endif

ifdef JOY_USB
CONFIG += -DUSB_JOYSTICK
ifeq ($(shell test -f /usr/include/usbhid.h && echo have_usbhid), have_usbhid)
CONFIG += -DHAVE_USBHID_H
MY_LIBS += -lusbhid
else
ifeq ($(shell test -f /usr/include/libusbhid.h && echo have_libusbhid), have_libusbhid)
CONFIG += -DHAVE_LIBUSBHID_H
MY_LIBS += -lusbhid
else
MY_LIBS += -lusb
endif
endif
endif

ifdef JOY_SDL
CONFIG  += -DSDL_JOYSTICK
#CONFIG  += -DSDL_JOYSTICK `$(SDL_CONFIG) --cflags`
#MY_LIBS += `$(SDL_CONFIG) --libs`
endif

# Happ UGCI config
ifdef UGCICOIN
CONFIG += -DUGCICOIN
MY_LIBS += -lugci
endif

ifdef LIGHTGUN_ABS_EVENT
CONFIG += -DUSE_LIGHTGUN_ABS_EVENT
endif

ifdef EFENCE
MY_LIBS += -lefence
endif

#we remove $(OBJ)/vidhrdw/vector.o from $(COREOBJS) since we have our own
#build rules for this object because it is display dependent.
OBJS += $(subst $(OBJ)/vidhrdw/vector.o, ,$(COREOBJS)) $(DRVLIBS)

#ASSEMBLER_OBJS = $(OBJ)/unix.$(DISPLAY_METHOD)/clut16to16.o $(OBJ)/unix.$(DISPLAY_METHOD)/memcpy.o

OSDEPEND = $(OBJDIR)/osdepend.a

VECTOR = $(OBJDIR)/vector.o


##############################################################################
# Start of the real makefile.
##############################################################################
$(NAME).$(DISPLAY_METHOD): $(ZLIB) $(OBJS) $(UNIX_OBJS) $(VECTOR) $(OSDEPEND) $(ASSEMBLER_OBJS)
	$(CC_COMMENT) @echo 'Linking $(NAME).$(DISPLAY_METHOD).$(MAME_VERSION) ...'
	$(CC_COMPILE) $(LD) $(LDFLAGS) -o $@ $(OBJS) $(UNIX_OBJS) $(OSDEPEND) $(VECTOR) $(ASSEMBLER_OBJS) $(MY_LIBS)
	@mv $(NAME).$(DISPLAY_METHOD) $(NAME).$(DISPLAY_METHOD).$(MAME_VERSION)

maketree: $(sort $(OBJDIRS))

$(sort $(OBJDIRS)):
	-mkdir -p $@

tools: $(ZLIB) $(TOOLS)

$(PLATFORM_IMGTOOL_OBJS):

xlistdev: src/unix/contrib/tools/xlistdev.c
	$(CC_COMMENT) @echo 'Compiling $< ...'
	$(CC_COMPILE) $(CC) $(X11INC) src/unix/contrib/tools/xlistdev.c -o xlistdev $(JSLIB) $(LIBS.$(ARCH)) $(LIBS.$(DISPLAY_METHOD)) -lXi -lm

romcmp: $(OBJ)/romcmp.o $(OBJ)/unzip.o
	$(CC_COMMENT) @echo Linking $@...
	$(CC_COMPILE) $(LD) $(LDFLAGS) -o $@ $^ -lz

chdman: $(OBJ)/chdman.o $(OBJ)/chd.o $(OBJ)/chdcd.o $(OBJ)/md5.o $(OBJ)/sha1.o $(OBJ)/version.o
	$(CC_COMMENT) @echo Linking $@...
	$(CC_COMPILE) $(LD) $(LDFLAGS) -o $@ $^ -lz

xml2info: src/xml2info/xml2info.c
	$(CC_COMMENT) @echo Compiling $@...
	$(CC_COMPILE) $(CC) -O1 -o $@ $^

dat2html: $(DAT2HTML_OBJS)
	$(CC_COMMENT) @echo Compiling $@...
	$(CC_COMPILE) $(LD) $(LDFLAGS) $^ -o $@

messdocs: 

imgtool: $(IMGTOOL_OBJS) $(PLATFORM_IMGTOOL_OBJS)
	$(CC_COMMENT) @echo Compiling $@...
	$(CC_COMPILE) $(LD) $(LDFLAGS) $^ -lz -o $@

messtest: $(OBJS) $(DRVLIBS) $(MESSTEST_OBJS) \
	$(OBJ)/mess/tools/messtest/messtest.o \
	$(OBJ)/mess/tools/messtest/testexec.o \
	$(OBJ)/mess/tools/messtest/tststubs.o \
	$(OBJ)/mess/tools/messtest/tstutils.o \
	$(OBJDIR)/dirio.o \
	$(OBJDIR)/fileio.o \
	$(OBJDIR)/ticker.o \
	$(OBJDIR)/parallel.o \
	$(OBJDIR)/sysdep/misc.o \
	$(OBJDIR)/sysdep/rc.o \
	$(OBJDIR)/tststubs.o \
	$(OBJDIR)/vector.o
	$(CC_COMMENT) @echo Linking $@...
	$(CC_COMPILE) $(LD) $(LDFLAGS) $(MY_LIBS) $^ -o $@

$(OBJDIR)/tststubs.o: src/unix/tststubs.c
	$(CC_COMPILE) $(CC) $(MY_CFLAGS) -o $@ -c $<

src/unix/contrib/cutzlib-1.2.1/libz.a:
	( \
	cd src/unix/contrib/cutzlib-1.2.1; \
	./configure; \
	$(MAKE) libz.a \
	)

ifdef MESS
$(OBJ)/mess/%.o: mess/%.c
	$(CC_COMMENT) @echo '[MESS] Compiling $< ...'
	$(CC_COMPILE) $(CC) $(MY_CFLAGS) -o $@ -c $<
endif

$(OBJ)/%.o: src/%.c
	$(CC_COMMENT) @echo 'Compiling $< ...'
	$(CC_COMPILE) $(CC) $(MY_CFLAGS) -o $@ -c $<

$(OBJ)/%.a:
	$(CC_COMMENT) @echo 'Archiving $@ ...'
	$(CC_COMPILE) ar $(AR_OPTS) $@ $^
	$(CC_COMPILE) $(RANLIB) $@

$(OSDEPEND): $(UNIX_OBJS)
	$(CC_COMMENT) @echo 'Archiving $@ ...'
	$(CC_COMPILE) ar $(AR_OPTS) $@ $(UNIX_OBJS)
	$(CC_COMPILE) $(RANLIB) $@

$(UNIX_OBJDIR)/%.o: src/unix/%.c src/unix/xmame.h
	$(CC_COMMENT) @echo '[OSEPEND] Compiling $< ...'
	$(CC_COMPILE) $(CC) $(CONFIG) -o $@ -c $<

$(UNIX_OBJDIR)/%.o: %.m src/unix/xmame.h
	$(CC_COMMENT) @echo '[OSEPEND] Compiling $< ...'
	$(CC_COMPILE) $(CC) $(MY_CFLAGS) -o $@ -c $<

# special cases for the 68000 core
#
# compile generated C files for the 68000 emulator
$(M68000_GENERATED_OBJS): $(OBJ)/cpu/m68000/m68kmake
	$(CC_COMMENT) @echo Compiling $(subst .o,.c,$@)...
	$(CC_COMPILE) $(CC) $(MY_CFLAGS) -c $*.c -o $@

# additional rule, because m68kcpu.c includes the generated m68kops.h :-/
$(OBJ)/cpu/m68000/m68kcpu.o: $(OBJ)/cpu/m68000/m68kmake

# generate C source files for the 68000 emulator
$(OBJ)/cpu/m68000/m68kmake: src/cpu/m68000/m68kmake.c
	$(CC_COMMENT) @echo M68K make $<...
	$(CC_COMPILE) $(CC) $(MY_CFLAGS) -DDOS -o $(OBJ)/cpu/m68000/m68kmake $<
	$(CC_COMMENT) @echo Generating M68K source files...
	$(CC_COMPILE) $(OBJ)/cpu/m68000/m68kmake $(OBJ)/cpu/m68000 src/cpu/m68000/m68k_in.c

# generate asm source files for the 68000/68020 emulators
$(OBJ)/cpu/m68000/68000.asm:  src/cpu/m68000/make68k.c
	$(CC_COMMENT) @echo Compiling $<...
	$(CC_COMPILE) $(CC) $(MY_CFLAGS) -O0 -DDOS -o $(OBJ)/cpu/m68000/make68k $<
	$(CC_COMMENT) @echo Generating $@...
	$(CC_COMPILE) $(OBJ)/cpu/m68000/make68k $@ $(OBJ)/cpu/m68000/68000tab.asm 00

$(OBJ)/cpu/m68000/68020.asm:  src/cpu/m68000/make68k.c
	$(CC_COMMENT) @echo Compiling $<...
	$(CC_COMPILE) $(CC) $(MY_CFLAGS) -O0 -DDOS -o $(OBJ)/cpu/m68000/make68k $<
	$(CC_COMMENT) @echo Generating $@...
	$(CC_COMPILE) $(OBJ)/cpu/m68000/make68k $@ $(OBJ)/cpu/m68000/68020tab.asm 20

# generated asm files for the 68000 emulator
$(OBJ)/cpu/m68000/68000.o:  $(OBJ)/cpu/m68000/68000.asm
	$(CC_COMMENT) @echo Assembling $<...
	$(CC_COMPILE) $(ASM_STRIP) $<
	$(CC_COMPILE) nasm $(NASM_FMT) -o $@ $(subst -D,-d,$(ASMDEFS)) $<

$(OBJ)/cpu/m68000/68020.o:  $(OBJ)/cpu/m68000/68020.asm
	$(CC_COMMENT) @echo Assembling $<...
	$(CC_COMPILE) $(ASM_STRIP) $<
	$(CC_COMPILE) nasm $(NASM_FMT) -o $@ $(subst -D,-d,$(ASMDEFS)) $<

#some tricks, since vector.o these days is display method-dependent:
$(VECTOR): src/vidhrdw/vector.c
	$(CC_COMMENT) @echo 'Compiling $< ...'
	$(CC_COMPILE) $(CC) $(MY_CFLAGS) -o $@ -c $<

$(OBJ)/unix.$(DISPLAY_METHOD)/clut16to16.o: src/unix/clut16to16.s
	$(CC_COMMENT) @echo 'Assembling $< ...'
	$(CC_COMPILE) $(CC) $(MY_CFLAGS) -o $@ -c $<

$(OBJ)/unix.$(DISPLAY_METHOD)/memcpy.o: src/unix/memcpy.s
	$(CC_COMMENT) @echo 'Assembling $< ...'
	$(CC_COMPILE) $(CC) $(MY_CFLAGS) -o $@ -c $<


#make sure this isn't accidently in makefile.$(OBJ):
$(OBJ)/vidhrdw/vector.o: bla

doc: doc/xmame-doc.txt doc/x$(TARGET)rc.dist doc/gamelist.$(TARGET) doc/x$(TARGET).6

doc/xmame-doc.txt: doc/xmame-doc.sgml
	cd doc; \
	sgml2txt   -l en -p a4 -f          xmame-doc.sgml; \
	sgml2html  -l en -p a4             xmame-doc.sgml; \
	sgml2latex -l en -p a4 --output=ps xmame-doc.sgml; \
	rm -f xmame-doc.lyx~

doc/x$(TARGET)rc.dist: all src/unix/xmamerc-keybinding-notes.txt
	./x$(TARGET).$(DISPLAY_METHOD) -noloadconfig -showconfig | \
	 grep -v loadconfig | tr "\033" \# > doc/x$(TARGET)rc.dist
	cat src/unix/xmamerc-keybinding-notes.txt >> doc/x$(TARGET)rc.dist

doc/gamelist.$(TARGET): all
	./x$(TARGET).$(DISPLAY_METHOD) -listgamelistheader > doc/gamelist.$(TARGET)
	./x$(TARGET).$(DISPLAY_METHOD) -listgamelist >> doc/gamelist.$(TARGET)

doc/x$(TARGET).6: all src/unix/xmame.6-1 src/unix/xmame.6-3
	cat src/unix/xmame.6-1 > doc/x$(TARGET).6
	./x$(TARGET).$(DISPLAY_METHOD) -noloadconfig -manhelp | \
	 tr "\033" \# >> doc/x$(TARGET).6
	cat src/unix/xmame.6-3 >> doc/x$(TARGET).6

install: $(INST.$(DISPLAY_METHOD)) install-man
	@echo $(NAME) for $(ARCH)-$(MY_CPU) installation completed

install-man:
	@echo installing manual pages under $(MANDIR) ...
	-$(INSTALL_MAN_DIR) $(MANDIR)
	$(INSTALL_MAN) doc/x$(TARGET).6 $(MANDIR)/x$(TARGET).6

doinstall:
	@echo installing binaries under $(BINDIR)...
	-$(INSTALL_PROGRAM_DIR) $(BINDIR)
	$(INSTALL_PROGRAM) $(NAME).$(DISPLAY_METHOD) $(BINDIR)

doinstallsuid:
	@echo installing binaries under $(BINDIR)...
	-$(INSTALL_PROGRAM_DIR) $(BINDIR)
	$(INSTALL_PROGRAM_SUID) $(NAME).$(DISPLAY_METHOD) $(BINDIR)

copycab:
	@echo installing cabinet files under $(XMAMEROOT)...
	@for i in cab/*; do \
	if test ! -d $(XMAMEROOT)/$$i; then \
	$(INSTALL_DATA_DIR) $(XMAMEROOT)/$$i; fi; \
	for j in $$i/*; do $(INSTALL_DATA) $$j $(XMAMEROOT)/$$i; done; done

clean: 
	rm -fr $(OBJ) $(NAME).* xlistdev src/unix/contrib/cutzlib-1.2.1/libz.a src/unix/contrib/cutzlib-1.2.1/*.o $(TOOLS)
#	cd makedep; make clean

clean68k:
	@echo Deleting 68k files...
	rm -f $(OBJ)/cpuintrf.o
	rm -f $(OBJ)/drivers/cps2.o
	rm -rf $(OBJ)/cpu/m68000
