#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

# These set the information text in the nds file
GAME_TITLE     := XMXDS
GAME_SUBTITLE1 := xm / mod dj tool
GAME_SUBTITLE2 := based on libxm7

export TARGET	:=	$(shell basename $(GAME_TITLE))
export TOPDIR	:=	$(CURDIR)

# specify a directory which contains the nitro filesystem
# this is relative to the Makefile
NITRO_FILES	:= ./data
NITRO_FILES_DEMO	:= ./data_demo

include $(DEVKITARM)/ds_rules

.PHONY: checklibxm7 checkarm7 checkarm9 clean

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
all: checklibxm7 checkarm7 checkarm9 $(TARGET).nds $(TARGET)_demo.nds

#---------------------------------------------------------------------------------
checklibxm7:
	@$(MAKE) -C lib/libxm7

#---------------------------------------------------------------------------------
checkarm7:
	@$(MAKE) -C arm7
	
#---------------------------------------------------------------------------------
checkarm9:
	@$(MAKE) -C arm9

#---------------------------------------------------------------------------------
$(TARGET).nds	: $(NITRO_FILES) arm7/$(TARGET).elf arm9/$(TARGET).elf
	@echo "Assembling rom..."
	@ndstool	-c ../release/$(TARGET).nds -7 arm7/$(TARGET).elf -9 arm9/$(TARGET).elf \
	-b $(GAME_ICON) "$(GAME_TITLE);$(GAME_SUBTITLE1);$(GAME_SUBTITLE2)" \
	$(_ADDFILES)
    
    
$(TARGET)_demo.nds	: $(NITRO_FILES) arm7/$(TARGET).elf arm9/$(TARGET).elf
	@echo "Assembling demo rom..."
	@ndstool	-c ../release/$(TARGET).nds -7 arm7/$(TARGET).elf -9 arm9/$(TARGET).elf \
	-b $(GAME_ICON) "$(GAME_TITLE);$(GAME_SUBTITLE1);$(GAME_SUBTITLE2)" \
	$(_ADDFILES)
    

#---------------------------------------------------------------------------------
libxm7:
	$(MAKE) -C lib/libxm7

#---------------------------------------------------------------------------------
arm7/$(TARGET).elf:
	$(MAKE) -C arm7
	
#---------------------------------------------------------------------------------
arm9/$(TARGET).elf:
	$(MAKE) -C arm9

#---------------------------------------------------------------------------------
clean:
	$(MAKE) -C lib/libxm7 clean
	$(MAKE) -C arm9 clean
	$(MAKE) -C arm7 clean
	rm -f ../release/$(TARGET).nds $(TARGET).arm7 $(TARGET).arm9
