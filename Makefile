# If RACK_DIR is not defined when calling the Makefile, default to two directories above
RACK_DIR ?= ../..

# Must follow the format in the Naming section of
# https://vcvrack.com/manual/PluginDevelopmentTutorial.html
SLUG = Bargkass

# Must follow the format in the Versioning section of
# https://vcvrack.com/manual/PluginDevelopmentTutorial.html
VERSION = 0.6.0

# FLAGS will be passed to both the C and C++ compiler
FLAGS += -I src/deps/audiofile -I src/deps/osdialog
CFLAGS +=
CXXFLAGS +=

# Careful about linking to shared libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine.
LDFLAGS +=

# Add .cpp and .c files to the build
SOURCES += $(wildcard src/*.cpp) $(wildcard src/deps/audiofile/*.cpp) $(wildcard src/deps/kissfft/*.c)

# Add files to the ZIP package when running `make dist`
# The compiled plugin is automatically added.
DISTRIBUTABLES += $(wildcard LICENSE*) res

CFLAGS += $(shell pkg-config --cflags gtk+-2.0)
LDFLAGS += $(shell pkg-config --libs gtk+-2.0)
SOURCES += src/deps/osdialog/osdialog_gtk2.c

# Include the VCV Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk


# ifeq ($(ARCH),win)
# 	LDFLAGS += -lcomdlg32
# 	SOURCES += src/deps/osdialog/osdialog_win.c
# endif

# ifeq ($(ARCH),mac)
# 	LDFLAGS += -framework AppKit
# 	SOURCES += src/deps/osdialog/osdialog_mac.m
# endif
