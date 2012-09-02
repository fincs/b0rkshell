#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/base_rules

ASSETS := image
OUTDIR := cnv

VPATH    := $(foreach dir,$(ASSETS),$(CURDIR)/$(dir))
PNGFILES := $(foreach dir,$(ASSETS),$(notdir $(wildcard $(dir)/*.grit)))
PNGFILES := $(PNGFILES:.grit=.png)
GRFFILES := $(foreach item,$(PNGFILES:.png=.grf),$(OUTDIR)/$(item))

.PHONY: all

all: $(GRFFILES)

$(GRFFILES): $(OUTDIR)/%.grf : %.png
	@mkdir -p $(OUTDIR)
	@grit $< -ftr -fh! -o $@
