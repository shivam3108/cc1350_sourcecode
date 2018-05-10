COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR ?= $(abspath ../../../../../../..)

include $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/imports.mak

XDCPATH = $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source;$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/kernel/tirtos/packages;

CC = "$(IAR_ARMCOMPILER)/bin/iccarm"
LNK = "$(IAR_ARMCOMPILER)/bin/ilinkarm"

XDCTARGET = iar.targets.arm.M3
PLATFORM = ti.platforms.simplelink:CC1350F128

OBJECTS = oad_client.obj oad_protocol.obj oad_storage.obj oad_target_external_flash.obj SceAdc.obj EasyLink.obj ExtFlash.obj smartrf_settings.obj smartrf_settings_predefined.obj NodeTask.obj rfWsnNode.obj NodeRadioTask.obj scif_framework.obj scif_osal_tirtos.obj scif.obj CC1350_LAUNCHXL.obj

CONFIGPKG = rfWsnNodeExtFlashOadClient

NAME = rfWsnNodeExtFlashOadClient

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
    "$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/devices/cc13x0/driverlib/bin/iar/driverlib.lib" \
    --config ../../tirtos/iar/CC1350_LAUNCHXL_TIRTOS_OAD.icf \
    --map "$(NAME).map" \
    --silent \
    --cpu=Cortex-M3 \
    --redirect _Printf=_PrintfSmall \
    --redirect _Scanf=_ScanfSmall \
    --semihosting=iar_breakpoint

.PRECIOUS: $(CONFIGPKG)/compiler.opt $(CONFIGPKG)/linker.cmd

all: $(NAME).out

$(CONFIGPKG)/compiler.opt: $(CONFIGPKG)/linker.cmd

$(CONFIGPKG)/linker.cmd: ../../rfWsnNodeExtFlashOadClient.cfg
	@ echo Running Configuro...
	@ $(XDC_INSTALL_DIR)/xs --xdcpath="$(XDCPATH)" xdc.tools.configuro -c "$(IAR_ARMCOMPILER)" -t $(XDCTARGET) -p $(PLATFORM) --compileOptions "$(CFLAGS)" -o "$(CONFIGPKG)" ../../rfWsnNodeExtFlashOadClient.cfg

oad_client.obj: ../../oad/native_oad/oad_client.c $(CONFIGPKG)/compiler.opt
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

SceAdc.obj: ../../SceAdc.c $(CONFIGPKG)/compiler.opt
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

CC1350_LAUNCHXL.obj: ../../CC1350_LAUNCHXL.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -f $(CONFIGPKG)/compiler.opt -o $@

$(NAME).out: $(OBJECTS) $(CONFIGPKG)/linker.cmd
	@ echo linking...
	@ $(LNK)  $(OBJECTS) -f $(CONFIGPKG)/linker.cmd $(LFLAGS) -o $(NAME).out

clean:
	@ echo Cleaning...
	@ $(RM) $(OBJECTS) > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).out > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).map > $(DEVNULL) 2>&1
	@ $(RMDIR) $(NAME) > $(DEVNULL) 2>&1
