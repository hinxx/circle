#
# Makefile
#

CIRCLEHOME = ../../..

OBJS	= main.o kernel.o triangle2.o

LIBS	= $(CIRCLEHOME)/addon/imgui/libimgui.a \
	  $(CIRCLEHOME)/addon/vc4/interface/bcm_host/libbcm_host.a \
	  $(CIRCLEHOME)/addon/vc4/interface/khronos/libkhrn_client.a \
	  $(CIRCLEHOME)/addon/vc4/interface/vmcs_host/libvmcs_host.a \
	  $(CIRCLEHOME)/addon/vc4/interface/vcos/libvcos.a \
	  $(CIRCLEHOME)/addon/vc4/vchiq/libvchiq.a \
	  $(CIRCLEHOME)/addon/linux/liblinuxemu.a \
	  $(CIRCLEHOME)/lib/sched/libsched.a \
	  $(CIRCLEHOME)/lib/usb/libusb.a \
	  $(CIRCLEHOME)/lib/input/libinput.a \
	  $(CIRCLEHOME)/lib/fs/libfs.a \
	  $(CIRCLEHOME)/lib/libcircle.a

include $(CIRCLEHOME)/Rules.mk

# XXX: from vc4 Makefile; is it needed??
CFLAGS += -Wno-strict-overflow
DEFINE += -DEGL_SERVER_DISPMANX

DEFINE += -DRPI
CFLAGS += -DIMGUI_IMPL_OPENGL_ES2

#INCLUDE	+= -I..

-include $(DEPS)
