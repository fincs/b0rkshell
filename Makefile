#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>devkitPro")
endif

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

ifeq ($(strip $(FEOSSDK)),)
$(error "Please set FEOSSDK in your environment. export FEOSSDK=<path to>FeOS/sdk")
endif

FEOSMK = $(FEOSSDK)/mk

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# DATA is a list of directories containing data files
# INCLUDES is a list of directories containing header files
#---------------------------------------------------------------------------------
TARGET        := $(shell basename $(CURDIR))
BUILD         := build
SOURCES       := source
DATA          := data
INCLUDES      := include

MANIFEST      := package.manifest
PACKAGENAME   := $(TARGET)

CONF_DEFINES =
CONF_USERLIBS = feosclasses libdecompress coopgui
CONF_LIBS = -lfeosclasses -llibdecompress -lcoopgui
CONF_PREREQUISITES = assets
CONF_EXTRACLEAN = cnv

include $(FEOSMK)/app.mk
include $(FEOSMK)/package.mk

.PHONY: install assets

install: all
	@mkdir -p $(FEOSDEST)/data/FeOS/bin
	@mkdir -p $(FEOSDEST)/data/FeOS/gui/assets
	@mkdir -p $(FEOSDEST)/data/FeOS/gui/fonts
	@cp $(TARGET).fx2 $(FEOSDEST)/data/FeOS/bin/$(TARGET).fx2
	@cp cnv/*.grf $(FEOSDEST)/data/FeOS/gui/assets/
	@cp tahoma10.ffnt $(FEOSDEST)/data/FeOS/gui/fonts/tahoma10.ffnt

assets:
	@echo Converting assets...
	@$(MAKE) --no-print-directory -f assets.mk
