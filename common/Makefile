ifeq ($(strip $(DEVKITPRO)),)
    $(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif

# -----------------------------------------------

TARGET            =    lib$(notdir $(CURDIR))
EXTENSION         =    a
OUT               =    lib
BUILD             =    build
SOURCES           =    src
INCLUDES          =    include ../lib/libstratosphere/libstratosphere/include
LIBS              =    ../lib/inih

DEFINES           =    __SWITCH__ NXLINK
ARCH              =    -march=armv8-a+crc+crypto+simd -mtune=cortex-a57 -mtp=soft -fpie
FLAGS             =    -Wall -pipe -g -O2 -ffunction-sections -fdata-sections                       \
                       -fno-stack-protector -fno-common
CFLAGS            =    -std=gnu11
CXXFLAGS          =    -std=gnu++20 -fno-rtti -fno-exceptions -fno-non-call-exceptions              \
                       -fno-threadsafe-statics -fno-use-cxa-atexit                                  \
                       -fno-asynchronous-unwind-tables -fno-unwind-tables
ASFLAGS           =
ARFLAGS           =    -rc

PREFIX            =    aarch64-none-elf-
CC                =    $(PREFIX)gcc
CXX               =    $(PREFIX)g++
AS                =    $(PREFIX)as
AR                =    $(PREFIX)gcc-ar
RANLIB            =    $(PREFIX)gcc-ranlib

# -----------------------------------------------

export PATH      :=    $(DEVKITPRO)/tools/bin:$(DEVKITPRO)/devkitA64/bin:$(PORTLIBS)/bin:$(PATH)

PORTLIBS          =    $(DEVKITPRO)/portlibs/switch
LIBNX             =    $(DEVKITPRO)/libnx
LIBS             :=    $(LIBS) $(LIBNX) $(PORTLIBS)

# -----------------------------------------------

CFILES            =    $(shell find $(SOURCES) -name *.c)
CPPFILES          =    $(shell find $(SOURCES) -name *.cpp)
SFILES            =    $(shell find $(SOURCES) -name *.s -or -name *.S)
OFILES            =    $(CFILES:%=$(BUILD)/%.o) $(CPPFILES:%=$(BUILD)/%.o) $(SFILES:%=$(BUILD)/%.o)
DFILES            =    $(OFILES:.o=.d)

LIB_TARGET        =    $(if $(OUT:=), $(OUT)/$(TARGET).$(EXTENSION), .$(OUT)/$(TARGET).$(EXTENSION))

DEFINE_FLAGS      =    $(addprefix -D,$(DEFINES))
INCLUDE_FLAGS     =    $(addprefix -I$(CURDIR)/,$(INCLUDES)) $(foreach dir,$(CUSTOM_LIBS),-I$(CURDIR)/$(dir)/include) \
                       $(foreach dir,$(filter-out $(CUSTOM_LIBS),$(LIBS)),-I$(dir)/include)

# -----------------------------------------------

.SUFFIXES:

.PHONY: all clean

all: $(LIB_TARGET)

$(LIB_TARGET): $(OFILES)
	@echo " AR  " $@
	@mkdir -p $(dir $@)
	@rm -f $@
	@$(AR) $(ARFLAGS) $@ $^
	@echo "Built" $(notdir $@)

$(BUILD)/%.c.o: %.c
	@echo " CC  " $@
	@mkdir -p $(dir $@)
	@$(CC) -MMD -MP $(ARCH) $(FLAGS) $(CFLAGS) $(DEFINE_FLAGS) $(INCLUDE_FLAGS) -c $(CURDIR)/$< -o $@

$(BUILD)/%.cpp.o: %.cpp
	@echo " CXX " $@
	@mkdir -p $(dir $@)
	@$(CXX) -MMD -MP $(ARCH) $(FLAGS) $(CXXFLAGS) $(DEFINE_FLAGS) $(INCLUDE_FLAGS) -c $(CURDIR)/$< -o $@

$(BUILD)/%.s.o: %.s %.S
	@echo " AS  " $@
	@mkdir -p $(dir $@)
	@$(AS) -MMD -MP -x assembler-with-cpp $(ARCH) $(FLAGS) $(ASFLAGS) $(INCLUDE_FLAGS) -c $(CURDIR)/$< -o $@

clean:
	@rm -rf $(BUILD) $(OUT)

-include $(DFILES)
