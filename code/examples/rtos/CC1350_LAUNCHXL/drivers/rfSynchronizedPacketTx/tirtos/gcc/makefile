COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR ?= $(abspath ../../../../../../..)

include $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/imports.mak

KERNEL_BUILD := $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/kernel/tirtos/builds/CC1350_LAUNCHXL/release

CC = "$(GCC_ARMCOMPILER)/bin/arm-none-eabi-gcc"
LNK = "$(GCC_ARMCOMPILER)/bin/arm-none-eabi-gcc"

OBJECTS = StateMachine.obj smartrf_settings.obj rfSynchronizedPacketTx.obj ccfg.obj CC1350_LAUNCHXL.obj

CONFIGPKG = $(KERNEL_BUILD)/gcc

NAME = rfSynchronizedPacketTx

CFLAGS = -I../.. \
    -DDeviceFamily_CC13X0 \
    -DCCFG_FORCE_VDDR_HH=0 \
    "-I$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source" \
    "-I$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/posix/gcc" \
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
    "-I$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/kernel/tirtos/packages/gnu/targets/arm/libs/install-native/arm-none-eabi/include/newlib-nano" \
    "-I$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/kernel/tirtos/packages/gnu/targets/arm/libs/install-native/arm-none-eabi/include" \
    "-I$(GCC_ARMCOMPILER)/arm-none-eabi/include"

LFLAGS = -Wl,-T,../../tirtos/gcc/CC1350_LAUNCHXL_TIRTOS.lds \
    "-Wl,-Map,$(NAME).map" \
    "-L$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/display/lib" \
    -l:display.am3g \
    "-L$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/grlib/lib/gcc/m3" \
    -l:grlib.a \
    "-L$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/drivers/rf/lib" \
    -l:rf_multiMode_cc13x0.am3g \
    "-L$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/drivers/lib" \
    -l:drivers_cc13x0.am3g \
    "-L$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/drivers/pdm/lib" \
    -l:pdm_cc13x0.am3g \
    "-L$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/kernel/tirtos/packages/ti/dpl/lib" \
    -l:dpl_cc13x0.am3g \
    "-Wl,-T,$(KERNEL_BUILD)/gcc/linker.cmd" \
    "-L$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/devices/cc13x0/driverlib/bin/gcc" \
    -l:driverlib.lib \
    -march=armv7-m \
    -mthumb \
    -nostartfiles \
    -static \
    -Wl,--gc-sections \
    "-L$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/kernel/tirtos/packages/gnu/targets/arm/libs/install-native/arm-none-eabi/lib/thumb/v7-m" \
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

StateMachine.obj: ../../StateMachine.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c @$(CONFIGPKG)/compiler.opt -o $@

smartrf_settings.obj: ../../smartrf_settings/smartrf_settings.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c @$(CONFIGPKG)/compiler.opt -o $@

rfSynchronizedPacketTx.obj: ../../rfSynchronizedPacketTx.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c @$(CONFIGPKG)/compiler.opt -o $@

ccfg.obj: ../../ccfg.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c @$(CONFIGPKG)/compiler.opt -o $@

CC1350_LAUNCHXL.obj: ../../CC1350_LAUNCHXL.c $(CONFIGPKG)/compiler.opt
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
