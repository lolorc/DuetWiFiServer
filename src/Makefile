# Makefile for DuetWiFiServer
# written by Christian Hammacher
#
# Licensed under the terms of the GNU Public License v3
# see http://www.gnu.org/licenses/gpl-3.0.html
#

# Referenced component versions
ARDUINO_VERSION := 1.6.12
GCC_VERSION := 1.20.0-26-gb404fb9-2
ESP8266_BOARD_VERSION := 2.3.0
ESPTOOL_VERSION := 0.4.9

# Workspace paths
LD_PATH = $(PWD)/ld
BUILD_PATH := $(PWD)/../Release/obj
OUTPUT_PATH := $(PWD)/../Release

# Compiler options
OPTIMIZATION := -Os

# ================================ Prerequisites ====================================

# Determine Arduino path
UNAME := $(shell uname -s)
ifeq ($(UNAME),Linux)
  ARDUINO_PATH := $(HOME)/.arduino15
endif
ifeq ($(UNAME),Darwin)
  ARDUINO_PATH := $(HOME)/Library/Arduino15
endif
ifeq (,$(wildcard $(ARDUINO_PATH)/.))
  $(error Arduino directory not found!)
endif

# Detect ESP8266 board path
ESP8266_BOARD_PATH := $(PWD)/../../CoreESP8266
ifeq (,$(wildcard $(ESP8266_BOARD_PATH)/.))
  $(error Arduino core for ESP8266 board not found!)
endif

# Detect library path
ESP8266_LIBRARY_PATH := $(PWD)/../../CoreESP8266/libraries
ifeq (,$(wildcard $(ESP8266_LIBRARY_PATH)/.))
  $(error ESP8266 libraries not found!)
endif

# Detect GCC path
GCC_PATH := $(ARDUINO_PATH)/packages/esp8266/tools/xtensa-lx106-elf-gcc/$(GCC_VERSION)
ifeq (,$(wildcard $(GCC_PATH)/.))
  $(error GCC toolchain not found!)
endif

# Detect esptool path
ESPTOOL := $(ARDUINO_PATH)/packages/esp8266/tools/esptool/$(ESPTOOL_VERSION)/esptool
ifeq (,$(wildcard $(ESPTOOL)))
  $(error esptool not found!)
endif

# ================================ GCC Options ======================================

CROSS_COMPILE := xtensa-lx106-elf-
CC := $(GCC_PATH)/bin/$(CROSS_COMPILE)gcc
CXX := $(GCC_PATH)/bin/$(CROSS_COMPILE)g++
LD := $(GCC_PATH)/bin/$(CROSS_COMPILE)gcc


INCLUDES := $(ESP8266_BOARD_PATH)/tools/sdk/include $(ESP8266_BOARD_PATH)/tools/sdk/lwip/include
INCLUDES += $(ESP8266_BOARD_PATH)/cores/esp8266 $(ESP8266_BOARD_PATH)/variants/nodemcu
INCLUDES += $(ESP8266_LIBRARY_PATH)/ESP8266WiFi/src $(ESP8266_LIBRARY_PATH)/ESP8266mDNS $(ESP8266_LIBRARY_PATH)/DNSServer/src $(ESP8266_LIBRARY_PATH)/EEPROM $(ESP8266_LIBRARY_PATH)/ESP8266SSDP

CFLAGS := -D__ets__ -DICACHE_FLASH -U__STRICT_ANSI__ -c -w -Wextra $(OPTIMIZATION) -g -Wpointer-arith -Wno-implicit-function-declaration -Wl,-EL -fno-inline-functions -nostdlib -mlongcalls -mtext-section-literals -falign-functions=4 -MMD -std=gnu99 -ffunction-sections -fdata-sections -DF_CPU=80000000L -DLWIP_OPEN_SRC -DARDUINO=10612 -DARDUINO_ESP8266_NODEMCU -DARDUINO_ARCH_ESP8266 -DARDUINO_BOARD=\"ESP8266_NODEMCU\" -DESP8266 -MMD -MP

CPPFLAGS := -D__ets__ -DICACHE_FLASH -U__STRICT_ANSI__ -c -w -Wextra $(OPTIMIZATION) -g -mlongcalls -mtext-section-literals -fno-exceptions -fno-rtti -falign-functions=4 -std=c++11 -MMD -ffunction-sections -fdata-sections -DF_CPU=80000000L -DLWIP_OPEN_SRC -DARDUINO=10612 -DARDUINO_ESP8266_NODEMCU -DARDUINO_ARCH_ESP8266 -DARDUINO_BOARD=\"ESP8266_NODEMCU\" -DESP8266 -MMD -MP

CFLAGS += $(foreach dir,$(INCLUDES),-I$(dir))
CPPFLAGS += $(foreach dir,$(INCLUDES),-I$(dir))

LDFLAGS := -g -Wall -Wextra $(OPTIMIZATION) -nostdlib -Wl,--no-check-sections -u call_user_start -Wl,-static "-L$(ESP8266_BOARD_PATH)/Release" "-L$(ESP8266_BOARD_PATH)/tools/sdk/lib" "-T$(ESP8266_BOARD_PATH)/tools/sdk/ld/eagle.flash.4m.ld" -Wl,--gc-sections -Wl,-wrap,system_restart_local -Wl,-wrap,register_chipv6_phy -Wl,--start-group -lm -lgcc -lhal -lphy -lpp -lnet80211 -lwpa -lcrypto -lmain -lwpa2 -lCoreESP8266 $(BUILD_PATH)/*.o -Wl,--end-group
# Currently unused libraries: wps axtls smartconfig mesh


# Unfortunately make doesn't support directory wildcards in targets, so instead we must explicitly specify the source paths by using VPATH
VPATH := $(PWD)

C_SOURCES += $(foreach dir,$(VPATH),$(wildcard $(dir)/*.c))
CPP_SOURCES := $(foreach dir,$(VPATH),$(wildcard $(dir)/*.cpp))

C_OBJS := $(foreach src,$(C_SOURCES),$(BUILD_PATH)/$(notdir $(src:.c=.c.o)))
CPP_OBJS := $(foreach src,$(CPP_SOURCES),$(BUILD_PATH)/$(notdir $(src:.cpp=.cpp.o)))

DEPS := $(C_OBJS:%.o=%.d) $(CPP_OBJS:%.o=%.d)


# ================================= Target all ======================================
.PHONY += all
all: $(OUTPUT_PATH)/DuetWiFiServer.bin
$(OUTPUT_PATH)/DuetWiFiServer.bin: $(OUTPUT_PATH)/DuetWiFiServer.elf
	@echo "  BIN     DuetWiFiServer.bin"
	@$(ESPTOOL) -eo "$(ESP8266_BOARD_PATH)/bootloaders/eboot/eboot.elf" -bo "$(OUTPUT_PATH)/DuetWiFiServer.bin" -bm dio -bf 40 -bz 4M -bs .text -bp 4096 -ec -eo "$(OUTPUT_PATH)/DuetWiFiServer.elf" -bs .irom0.text -bs .text -bs .data -bs .rodata -bc -ec

$(OUTPUT_PATH)/DuetWiFiServer.elf: $(BUILD_PATH) $(OUTPUT_PATH) $(C_OBJS) $(CPP_OBJS)
	@echo "  LD      DuetWiFiServer.elf"
	@$(LD) $(LDFLAGS) -o $(OUTPUT_PATH)/DuetWiFiServer.elf
-include $(DEPS)

$(BUILD_PATH)/%.c.o: %.c
	@echo "  CC      $(subst $(ESP8266_BOARD_PATH)/,,$(subst $(PWD)/,,$<))"
	@$(CC) $(CFLAGS) $< -o $@

$(BUILD_PATH)/%.cpp.o: %.cpp
	@echo "  CC      $(subst $(ESP8266_BOARD_PATH)/,,$(subst $(PWD)/,,$<))"
	@$(CXX) $(CPPFLAGS) $< -o $@

$(BUILD_PATH):
	@mkdir -p $(BUILD_PATH)

$(OUTPUT_PATH):
	@mkdir -p $(OUTPUT_PATH)


# ================================= Target clean ====================================
.PHONY += clean
clean:
	@rm -rf $(BUILD_PATH) $(OUTPUT_PATH)
	@rm -f $(PWD)/*.d
	$(info Build directories removed.)

