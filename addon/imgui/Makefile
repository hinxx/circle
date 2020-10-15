#
# Makefile
#

CIRCLEHOME = ../..

OBJS = imgui.o
#OBJS += imgui_impl_sdl.o
OBJS += imgui_impl_opengl3.o
OBJS += imgui_demo.o
OBJS += imgui_draw.o
OBJS += imgui_widgets.o

libimgui.a: $(OBJS)
	@echo "  AR    $@"
	@rm -f $@
	@$(AR) cr $@ $(OBJS)

include $(CIRCLEHOME)/Rules.mk

# XXX: from vc4 Makefile; is it needed??
CFLAGS += -Wno-strict-overflow
DEFINE += -DEGL_SERVER_DISPMANX

DEFINE += -DRPI
CFLAGS += -DIMGUI_IMPL_OPENGL_ES2
CFLAGS += -DIMGUI_DISABLE_FILE_FUNCTIONS
#CFLAGS += -DIMGUI_DISABLE_DEFAULT_FILE_FUNCTIONS

ifeq ($(strip $(AARCH)),64)
$(error AARCH = 64 is not supported imgui)
endif

ifeq ($(strip $(RASPPI)),4)
$(warning RASPPI = 4 is not supported in imgui)
endif

-include $(DEPS)
