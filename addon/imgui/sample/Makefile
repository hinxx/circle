#
# Makefile
#

include ../../../Config.mk

CIRCLEHOME = ../../..
NEWLIBDIR = ../../../stdlib/$(NEWLIB_ARCH)

#OBJS	= main.o kernel.o triangle2.o
OBJS	= main.o kernel.o imgui_demo.o

LIBS	= $(CIRCLEHOME)/addon/imgui/libimgui.a \
	  $(NEWLIBDIR)/lib/libm.a $(NEWLIBDIR)/lib/libc.a $(NEWLIBDIR)/lib/libcirclenewlib.a \
	  $(CIRCLEHOME)/addon/vc4/interface/bcm_host/libbcm_host.a \
	  $(CIRCLEHOME)/addon/vc4/interface/khronos/libkhrn_client.a \
	  $(CIRCLEHOME)/addon/vc4/interface/vmcs_host/libvmcs_host.a \
	  $(CIRCLEHOME)/addon/vc4/interface/vcos/libvcos.a \
	  $(CIRCLEHOME)/addon/vc4/vchiq/libvchiq.a \
	  $(CIRCLEHOME)/addon/linux/liblinuxemu.a \
	  $(CIRCLEHOME)/addon/fatfs/libfatfs.a \
	  $(CIRCLEHOME)/lib/sched/libsched.a \
	  $(CIRCLEHOME)/lib/usb/libusb.a \
	  $(CIRCLEHOME)/lib/input/libinput.a \
	  $(CIRCLEHOME)/lib/fs/libfs.a \
	  $(CIRCLEHOME)/lib/libcircle.a

include $(CIRCLEHOME)/Rules.mk

CFLAGS += -I $(NEWLIBDIR)/include -I $(STDDEF_INCPATH) -I ../../../include

# XXX: from vc4 Makefile; is it needed??
CFLAGS += -Wno-strict-overflow
DEFINE += -DEGL_SERVER_DISPMANX

DEFINE += -DRPI
CFLAGS += -DIMGUI_IMPL_OPENGL_ES2
CFLAGS += -DIMGUI_DISABLE_FILE_FUNCTIONS
#CFLAGS += -DIMGUI_DISABLE_DEFAULT_FILE_FUNCTIONS

#INCLUDE	+= -I..

-include $(DEPS)
