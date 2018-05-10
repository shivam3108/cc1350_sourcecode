SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR ?= $(abspath ../../../../../../..)

include $(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/imports.mak

KERNEL_BUILD := $(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/kernel/tirtos/builds/CC1350_LAUNCHXL/release

CC = "$(GCC_ARMCOMPILER)/bin/arm-none-eabi-gcc"
LNK = "$(GCC_ARMCOMPILER)/bin/arm-none-eabi-gcc"

OBJECTS = main_tirtos.obj pinStandby.obj CC1350_LAUNCHXL.obj ccfg.obj

CONFIGPKG = $(KERNEL_BUILD)/gcc

NAME = pinStandby

CFLAGS = -I../.. \
    -DDeviceFamily_CC13X0 \
    "-I$(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source" \
    "-I$(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/posix/gcc" \
    -mcpu=cortex-m3 \
    -march=armv7-m \
    -mthumb \
    -std=c99 \
    -mfloat-abi=soft \
    -ffunction-sections \
    -fdata-sections \
    -g \
    -gstrict-dwarf \
    -Wall \
    "-I$(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/kernel/tirtos/packages/gnu/targets/arm/libs/install-native/arm-none-eabi/include/newlib-nano" \
    "-I$(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/kernel/tirtos/packages/gnu/targets/arm/libs/install-native/arm-none-eabi/include" \
    "-I$(GCC_ARMCOMPILER)/arm-none-eabi/include"

LFLAGS = -Wl,-T,../../tirtos/gcc/CC1350_LAUNCHXL_TIRTOS.lds \
    "-Wl,-Map,$(NAME).map" \
    "-L$(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source" \
    -l:ti/display/lib/display.am3g \
    -l:ti/grlib/lib/gcc/m3/grlib.a \
    -l:third_party/spiffs/lib/gcc/m3/spiffs_cc26xx.a \
    -l:ti/drivers/rf/lib/rf_multiMode_cc13x0.am3g \
    -l:ti/drivers/lib/drivers_cc13x0.am3g \
    -l:ti/drivers/pdm/lib/pdm_cc13x0.am3g \
    "-L$(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/kernel/tirtos/packages" \
    -l:ti/dpl/lib/dpl_cc13x0.am3g \
    "-Wl,-T,$(KERNEL_BUILD)/gcc/linker.cmd" \
    -l:ti/devices/cc13x0/driverlib/bin/gcc/driverlib.lib \
    -march=armv7-m \
    -mthumb \
    -nostartfiles \
    -static \
    -Wl,--gc-sections \
    "-L$(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/kernel/tirtos/packages/gnu/targets/arm/libs/install-native/arm-none-eabi/lib/thumb/v7-m" \
    "-L$(GCC_ARMCOMPILER)/arm-none-eabi/lib" \
    -lgcc \
    -lc \
    -lm \
    -lnosys \
    --specs=nano.specs

all: $(NAME).out

$(CONFIGPKG)/linker.cmd $(CONFIGPKG)/compiler.opt:
	@ $(ECHOBLANKLINE)
	@ echo $(CONFIGPKG) is not built.
	@ echo You can build it by issuing $(MAKE) in $(CONFIGPKG).
	@ $(ECHOBLANKLINE)

main_tirtos.obj: ../../tirtos/main_tirtos.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c @$(CONFIGPKG)/compiler.opt -o $@

pinStandby.obj: ../../pinStandby.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c @$(CONFIGPKG)/compiler.opt -o $@

CC1350_LAUNCHXL.obj: ../../CC1350_LAUNCHXL.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c @$(CONFIGPKG)/compiler.opt -o $@

ccfg.obj: ../../ccfg.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c @$(CONFIGPKG)/compiler.opt -o $@

$(NAME).out: $(OBJECTS) $(CONFIGPKG)/linker.cmd
	@ echo linking...
	@ $(LNK)  $(OBJECTS) $(LFLAGS) -o $(NAME).out

clean:
	@ echo Cleaning...
	@ $(RM) $(OBJECTS) > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).out > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).map > $(DEVNULL) 2>&1
