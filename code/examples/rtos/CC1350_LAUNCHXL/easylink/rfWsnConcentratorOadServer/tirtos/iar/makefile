COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR ?= $(abspath ../../../../../../..)

include $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/imports.mak

KERNEL_BUILD := $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/kernel/tirtos/builds/CC1350_LAUNCHXL/release

CC = "$(IAR_ARMCOMPILER)/bin/iccarm"
LNK = "$(IAR_ARMCOMPILER)/bin/ilinkarm"

OBJECTS = oad_server.obj oad_protocol.obj oad_storage.obj oad_target_external_flash.obj oad_switch.obj EasyLink.obj ExtFlash.obj smartrf_settings.obj smartrf_settings_predefined.obj rfWsnConcentrator.obj ConcentratorRadioTask.obj ConcentratorTask.obj ccfg.obj CC1350_LAUNCHXL.obj

CONFIGPKG = $(KERNEL_BUILD)/iar

NAME = rfWsnConcentratorOadServer

CFLAGS = -I../.. \
    -DOAD_BLOCK_SIZE=64 \
    -DDeviceFamily_CC13X0 \
    -DCCFG_FORCE_VDDR_HH=0 \
    "-I$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source" \
    "-I$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/posix/iar" \
    --debug \
    --silent \
    -e \
    --aeabi \
    --thumb \
    --diag_suppress=Pa050

LFLAGS = "$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/display/lib/display.arm3" \
    "$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/grlib/lib/iar/m3/grlib.a" \
    "$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/drivers/rf/lib/rf_multiMode_cc13x0.arm3" \
    "$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/drivers/lib/drivers_cc13x0.arm3" \
    "$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/drivers/pdm/lib/pdm_cc13x0.arm3" \
    "$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/kernel/tirtos/packages/ti/dpl/lib/dpl_cc13x0.arm3" \
    -f "$(KERNEL_BUILD)/iar/linker.cmd" \
    "$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/devices/cc13x0/driverlib/bin/iar/driverlib.lib" \
    --config ../../tirtos/iar/CC1350_LAUNCHXL_TIRTOS.icf \
    --map "$(NAME).map" \
    --silent \
    --cpu=Cortex-M3 \
    --redirect _Printf=_PrintfSmall \
    --redirect _Scanf=_ScanfSmall \
    --semihosting=iar_breakpoint

all: $(NAME).out

$(CONFIGPKG)/linker.cmd $(CONFIGPKG)/compiler.opt:
	@ $(ECHOBLANKLINE)
	@ echo $(CONFIGPKG) is not built.
	@ echo You can build it by issuing $(MAKE) in $(CONFIGPKG).
	@ $(ECHOBLANKLINE)

oad_server.obj: ../../oad/native_oad/oad_server.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -f $(CONFIGPKG)/compiler.opt -o $@

oad_protocol.obj: ../../oad/native_oad/oad_protocol.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -f $(CONFIGPKG)/compiler.opt -o $@

oad_storage.obj: ../../oad/native_oad/oad_storage.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -f $(CONFIGPKG)/compiler.opt -o $@

oad_target_external_flash.obj: ../../oad/native_oad/oad_target_external_flash.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -f $(CONFIGPKG)/compiler.opt -o $@

oad_switch.obj: ../../oad/ble_oad/oad_switch.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -f $(CONFIGPKG)/compiler.opt -o $@

EasyLink.obj: ../../easylink/EasyLink.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -f $(CONFIGPKG)/compiler.opt -o $@

ExtFlash.obj: ../../extflash/ExtFlash.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -f $(CONFIGPKG)/compiler.opt -o $@

smartrf_settings.obj: ../../smartrf_settings/smartrf_settings.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -f $(CONFIGPKG)/compiler.opt -o $@

smartrf_settings_predefined.obj: ../../smartrf_settings/smartrf_settings_predefined.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -f $(CONFIGPKG)/compiler.opt -o $@

rfWsnConcentrator.obj: ../../rfWsnConcentrator.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -f $(CONFIGPKG)/compiler.opt -o $@

ConcentratorRadioTask.obj: ../../ConcentratorRadioTask.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -f $(CONFIGPKG)/compiler.opt -o $@

ConcentratorTask.obj: ../../ConcentratorTask.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -f $(CONFIGPKG)/compiler.opt -o $@

ccfg.obj: ../../ccfg.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -f $(CONFIGPKG)/compiler.opt -o $@

CC1350_LAUNCHXL.obj: ../../CC1350_LAUNCHXL.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -f $(CONFIGPKG)/compiler.opt -o $@

$(NAME).out: $(OBJECTS) $(CONFIGPKG)/linker.cmd
	@ echo linking...
	@ $(LNK)  $(OBJECTS) $(LFLAGS) -o $(NAME).out

clean:
	@ echo Cleaning...
	@ $(RM) $(OBJECTS) > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).out > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).map > $(DEVNULL) 2>&1
