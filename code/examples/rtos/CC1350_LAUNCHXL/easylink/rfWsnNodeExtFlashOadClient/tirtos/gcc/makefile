COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR ?= $(abspath ../../../../../../..)

include $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/imports.mak

XDCPATH = $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source;$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/kernel/tirtos/packages;

CC = "$(GCC_ARMCOMPILER)/bin/arm-none-eabi-gcc"
LNK = "$(GCC_ARMCOMPILER)/bin/arm-none-eabi-gcc"

XDCTARGET = gnu.targets.arm.M3
PLATFORM = ti.platforms.simplelink:CC1350F128

OBJECTS = oad_client.obj oad_protocol.obj oad_storage.obj oad_target_external_flash.obj SceAdc.obj EasyLink.obj ExtFlash.obj smartrf_settings.obj smartrf_settings_predefined.obj NodeTask.obj rfWsnNode.obj NodeRadioTask.obj scif_framework.obj scif_osal_tirtos.obj scif.obj CC1350_LAUNCHXL.obj

CONFIGPKG = rfWsnNodeExtFlashOadClient

NAME = rfWsnNodeExtFlashOadClient

CFLAGS = -I../.. \
    -DOAD_BLOCK_SIZE=64 \
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

LFLAGS = -Wl,-T,../../tirtos/gcc/CC1350_LAUNCHXL_TIRTOS_OAD.lds \
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
    "-Wl,-T,$(NAME)/linker.cmd" \
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

.PRECIOUS: $(CONFIGPKG)/compiler.opt $(CONFIGPKG)/linker.cmd

all: $(NAME).out

$(CONFIGPKG)/compiler.opt: $(CONFIGPKG)/linker.cmd

$(CONFIGPKG)/linker.cmd: ../../rfWsnNodeExtFlashOadClient.cfg
	@ echo Running Configuro...
	@ $(XDC_INSTALL_DIR)/xs --xdcpath="$(XDCPATH)" xdc.tools.configuro -c "$(GCC_ARMCOMPILER)" -t $(XDCTARGET) -p $(PLATFORM) --compileOptions "$(CFLAGS)" -o "$(CONFIGPKG)" ../../rfWsnNodeExtFlashOadClient.cfg

oad_client.obj: ../../oad/native_oad/oad_client.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c @$(CONFIGPKG)/compiler.opt -o $@

oad_protocol.obj: ../../oad/native_oad/oad_protocol.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c @$(CONFIGPKG)/compiler.opt -o $@

oad_storage.obj: ../../oad/native_oad/oad_storage.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c @$(CONFIGPKG)/compiler.opt -o $@

oad_target_external_flash.obj: ../../oad/native_oad/oad_target_external_flash.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c @$(CONFIGPKG)/compiler.opt -o $@

SceAdc.obj: ../../SceAdc.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c @$(CONFIGPKG)/compiler.opt -o $@

EasyLink.obj: ../../easylink/EasyLink.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c @$(CONFIGPKG)/compiler.opt -o $@

ExtFlash.obj: ../../extflash/ExtFlash.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c @$(CONFIGPKG)/compiler.opt -o $@

smartrf_settings.obj: ../../smartrf_settings/smartrf_settings.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c @$(CONFIGPKG)/compiler.opt -o $@

smartrf_settings_predefined.obj: ../../smartrf_settings/smartrf_settings_predefined.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c @$(CONFIGPKG)/compiler.opt -o $@

NodeTask.obj: ../../NodeTask.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c @$(CONFIGPKG)/compiler.opt -o $@

rfWsnNode.obj: ../../rfWsnNode.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c @$(CONFIGPKG)/compiler.opt -o $@

NodeRadioTask.obj: ../../NodeRadioTask.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c @$(CONFIGPKG)/compiler.opt -o $@

scif_framework.obj: ../../sce/scif_framework.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c @$(CONFIGPKG)/compiler.opt -o $@

scif_osal_tirtos.obj: ../../sce/scif_osal_tirtos.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c @$(CONFIGPKG)/compiler.opt -o $@

scif.obj: ../../sce/scif.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c @$(CONFIGPKG)/compiler.opt -o $@

CC1350_LAUNCHXL.obj: ../../CC1350_LAUNCHXL.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c @$(CONFIGPKG)/compiler.opt -o $@

$(NAME).out: $(OBJECTS) $(CONFIGPKG)/linker.cmd
	@ echo linking...
	@ $(LNK)  $(OBJECTS)  $(LFLAGS) -o $(NAME).out

clean:
	@ echo Cleaning...
	@ $(RM) $(OBJECTS) > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).out > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).map > $(DEVNULL) 2>&1
	@ $(RMDIR) $(NAME) > $(DEVNULL) 2>&1
