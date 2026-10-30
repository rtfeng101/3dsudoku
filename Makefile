#---------------------------------------------------------------------------------
# sudoku Makefile
#
#   make        -> builds sudoku.3dsx  (run it with 3dslink or the Homebrew Launcher)
#   make cia    -> builds sudoku.3dsx AND sudoku.cia  (install with FBI)
#   make clean  -> deletes everything that was built
#
# The top half is devkitPro's standard 3DS template (trimmed down).
# The CIA part is at the bottom, with each step explained.
#---------------------------------------------------------------------------------
.SUFFIXES:

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITARM)/3ds_rules

#---------------------------------------------------------------------------------
# App info -- change these for your own apps
#---------------------------------------------------------------------------------
TARGET          := sudoku
APP_TITLE       := Sudoku
APP_DESCRIPTION := Sudoku for the 3DS
APP_AUTHOR      := Me

# Icon used for BOTH the .3dsx (Homebrew Launcher) and the .cia (HOME menu).
# Must be a 48x48 PNG.
ICON            := meta/icon.png

# --- CIA-only settings ---
# Every installed title needs its own Unique ID. If two CIAs share one,
# installing the second replaces the first. Homebrew uses 0xF8000-0xFFFFF.
# Pick a new random value in that range for each new app.
CIA_UNIQUE_ID   := 0xFD6B0
# Product code: any short ID. Shown in FBI's title list.
CIA_PRODUCT_CODE:= CTR-H-SDKU
# Internal process name. 8 characters max, no spaces.
CIA_PROCESS_NAME:= sudoku
# Version shown in FBI / System Settings (major.minor.micro).
CIA_VERSION_MAJOR := 1
CIA_VERSION_MINOR := 0
CIA_VERSION_MICRO := 0

# Banner = what the top screen shows when the app is highlighted on the HOME menu.
# Image must be a 256x128 PNG. Audio is a short WAV or OGG (keep it under ~3 seconds).
CIA_BANNER_IMAGE:= meta/banner.png
CIA_BANNER_AUDIO:= meta/audio.wav
# Permissions and settings for the installed app.
CIA_RSF         := meta/app.rsf

# Tool names. Override if they live somewhere else, e.g. make cia MAKEROM=~/bin/makerom
MAKEROM         ?= makerom
BANNERTOOL      ?= bannertool

#---------------------------------------------------------------------------------
# Folders
#---------------------------------------------------------------------------------
BUILD           := build
SOURCES         := source
INCLUDES        := include

#---------------------------------------------------------------------------------
# Compiler options
#---------------------------------------------------------------------------------
ARCH     := -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft

CFLAGS   := -g -Wall -O2 -mword-relocations -ffunction-sections $(ARCH)
CFLAGS   += $(INCLUDE) -D__3DS__
CXXFLAGS := $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++17
ASFLAGS  := -g $(ARCH)
LDFLAGS   = -specs=3dsx.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)

LIBS     := -lctru -lm
LIBDIRS  := $(CTRULIB)

#---------------------------------------------------------------------------------
# Outer pass: runs in the project folder, then re-runs make inside build/
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT   := $(CURDIR)/$(TARGET)
export TOPDIR   := $(CURDIR)
export VPATH    := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
export DEPSDIR  := $(CURDIR)/$(BUILD)

CFILES          := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES        := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES          := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))

# Link with g++ only if there are C++ files.
ifeq ($(strip $(CPPFILES)),)
	export LD := $(CC)
else
	export LD := $(CXX)
endif

export OFILES   := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)

export INCLUDE  := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
                   $(foreach dir,$(LIBDIRS),-I$(dir)/include) \
                   -I$(CURDIR)/$(BUILD)

export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

export APP_ICON := $(TOPDIR)/$(ICON)
export _3DSXFLAGS += --smdh=$(CURDIR)/$(TARGET).smdh

.PHONY: all clean cia check-cia-tools

#---------------------------------------------------------------------------------
# make  ->  compile the code, link sudoku.elf, wrap it as sudoku.3dsx
#---------------------------------------------------------------------------------
all:
	@mkdir -p $(BUILD)
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@echo "Cleaning..."
	@rm -fr $(BUILD) $(TARGET).3dsx $(TARGET).smdh $(TARGET).elf $(TARGET).cia

#---------------------------------------------------------------------------------
# make cia  ->  everything above, plus 3 packaging steps:
#
#   1. bannertool makesmdh   meta/icon.png  -> build/icon.icn
#        The HOME menu icon + the name/description/author text.
#
#   2. bannertool makebanner meta/banner.png + meta/audio.wav -> build/banner.bnr
#        The picture and sound shown on the top screen when the app is selected.
#
#   3. makerom -f cia        sudoku.elf + icon + banner + meta/app.rsf -> sudoku.cia
#        Packs the compiled program (the same .elf used for the .3dsx) together
#        with the icon and banner. app.rsf says what the app is allowed to do
#        (memory, system services, etc.). "-target t" makes a fake-signed
#        homebrew CIA, which Luma3DS lets you install.
#---------------------------------------------------------------------------------
cia: check-cia-tools all
	@echo ""
	@echo "[CIA 1/3] Making HOME menu icon:   $(ICON) -> $(BUILD)/icon.icn"
	@$(BANNERTOOL) makesmdh \
		-s "$(APP_TITLE)" \
		-l "$(APP_DESCRIPTION)" \
		-p "$(APP_AUTHOR)" \
		-i $(ICON) \
		-o $(BUILD)/icon.icn
	@echo "[CIA 2/3] Making HOME menu banner: $(CIA_BANNER_IMAGE) + $(CIA_BANNER_AUDIO) -> $(BUILD)/banner.bnr"
	@$(BANNERTOOL) makebanner \
		-i $(CIA_BANNER_IMAGE) \
		-a $(CIA_BANNER_AUDIO) \
		-o $(BUILD)/banner.bnr
	@echo "[CIA 3/3] Packaging:               $(TARGET).elf -> $(TARGET).cia"
	@$(MAKEROM) -f cia -target t \
		-o $(TARGET).cia \
		-elf $(TARGET).elf \
		-rsf $(CIA_RSF) \
		-icon $(BUILD)/icon.icn \
		-banner $(BUILD)/banner.bnr \
		-major $(CIA_VERSION_MAJOR) -minor $(CIA_VERSION_MINOR) -micro $(CIA_VERSION_MICRO) \
		-DAPP_PROCESS_NAME=$(CIA_PROCESS_NAME) \
		-DAPP_PRODUCT_CODE=$(CIA_PRODUCT_CODE) \
		-DAPP_UNIQUE_ID=$(CIA_UNIQUE_ID)
	@echo ""
	@echo "Done! Built $(TARGET).cia ($$(du -h $(TARGET).cia | cut -f1))"
	@echo "Copy it to your SD card and install it with FBI."

# Friendly error if makerom or bannertool isn't installed.
check-cia-tools:
	@command -v $(MAKEROM) >/dev/null 2>&1 || { echo "Error: '$(MAKEROM)' not found. Install it (Step 3) or run: make cia MAKEROM=/path/to/makerom"; exit 1; }
	@command -v $(BANNERTOOL) >/dev/null 2>&1 || { echo "Error: '$(BANNERTOOL)' not found. Install it (Step 4) or run: make cia BANNERTOOL=/path/to/bannertool"; exit 1; }

#---------------------------------------------------------------------------------
# Inner pass: runs inside build/ and does the actual compiling
#---------------------------------------------------------------------------------
else

DEPENDS := $(OFILES:.o=.d)

$(OUTPUT).3dsx : $(OUTPUT).elf $(OUTPUT).smdh
$(OUTPUT).elf  : $(OFILES)

-include $(DEPENDS)

endif
