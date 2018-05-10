COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR ?= $(abspath ../../../../../../..)

include $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/imports.mak

KERNEL_BUILD := $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/kernel/tirtos/builds/CC1350_LAUNCHXL/release

CC = "$(IAR_ARMCOMPILER)/bin/iccarm"
LNK = "$(IAR_ARMCOMPILER)/bin/ilinkarm"

OBJECTS = SceAdc.obj EasyLink.obj BleAdv.obj urfc.obj uble.obj ugap.obj ull.obj urfi.obj smartrf_settings.obj smartrf_settings_predefined.obj NodeTask.obj rfWsnNode.obj NodeRadioTask.obj scif_framework.obj scif_osal_tirtos.obj scif.obj ccfg.obj CC1350_LAUNCHXL.obj

CONFIGPKG = $(KERNEL_BUILD)/iar

NAME = rfWsnNodeBleAdv

CFLAGS = -I../.. \
    -DBOARD_DISPLAY_USE_LCD=1 \
    -DFEATURE_BROADCASTER \
    -DFEATURE_ADVERTISER \
    -DCC13XX_LAUNCHXL \
    -DRF_MULTI_MODE \
    -DFEATURE_BLE_ADV \
    "-I$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/blestack/boards/" \
    "-I$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/blestack/inc/" \
    "-I$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/blestack/controller/cc26xx/inc" \
    "-I$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/blestack/hal/src/target/_common" \
    "-I$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/blestack/hal/src/target/_common/cc13xx" \
    "-I$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/blestack/hal/src/inc" \
    "-I$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/blestack/osal/src/inc" \
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

SceAdc.obj: ../../SceAdc.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -f $(CONFIGPKG)/compiler.opt -o $@

EasyLink.obj: ../../easylink/EasyLink.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -f $(CONFIGPKG)/compiler.opt -o $@

BleAdv.obj: ../../ble_adv/BleAdv.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -f $(CONFIGPKG)/compiler.opt -o $@

urfc.obj: ../../ble_adv/urfc.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -f $(CONFIGPKG)/compiler.opt -o $@

uble.obj: ../../ble_adv/uble.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -f $(CONFIGPKG)/compiler.opt -o $@

ugap.obj: ../../ble_adv/ugap.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -f $(CONFIGPKG)/compiler.opt -o $@

ull.obj: ../../ble_adv/ull.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -f $(CONFIGPKG)/compiler.opt -o $@

urfi.obj: ../../ble_adv/urfi.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -f $(CONFIGPKG)/compiler.opt -o $@

smartrf_settings.obj: ../../smartrf_settings/smartrf_settings.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -f $(CONFIGPKG)/compiler.opt -o $@

smartrf_settings_predefined.obj: ../../smartrf_settings/smartrf_settings_predefined.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -f $(CONFIGPKG)/compiler.opt -o $@

NodeTask.obj: ../../NodeTask.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -f $(CONFIGPKG)/compiler.opt -o $@

rfWsnNode.obj: ../../rfWsnNode.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -f $(CONFIGPKG)/compiler.opt -o $@

NodeRadioTask.obj: ../../NodeRadioTask.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -f $(CONFIGPKG)/compiler.opt -o $@

scif_framework.obj: ../../sce/scif_framework.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -f $(CONFIGPKG)/compiler.opt -o $@

scif_osal_tirtos.obj: ../../sce/scif_osal_tirtos.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -f $(CONFIGPKG)/compiler.opt -o $@

scif.obj: ../../sce/scif.c $(CONFIGPKG)/compiler.opt
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
