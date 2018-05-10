COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR ?= $(abspath ../../../../../../..)

include $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/imports.mak

KERNEL_BUILD := $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/kernel/tirtos/builds/CC1350_LAUNCHXL/release

CC = "$(CCS_ARMCOMPILER)/bin/armcl"
LNK = "$(CCS_ARMCOMPILER)/bin/armcl"

OBJECTS = EasyLink.obj AtControl.obj AtParams.obj AtProcess.obj AtTerm.obj smartrf_settings.obj smartrf_settings_predefined.obj Per.obj rfEasyLinkNp.obj ccfg.obj CC1350_LAUNCHXL.obj

CONFIGPKG = $(KERNEL_BUILD)/ccs

NAME = rfEasyLinkNp

CFLAGS = -I../.. \
    -DDeviceFamily_CC13X0 \
    -DCCFG_FORCE_VDDR_HH=0 \
    "-I$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source" \
    "-I$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/posix/ccs" \
    --silicon_version=7M3 \
    --code_state=16 \
    --little_endian \
    --display_error_number \
    --diag_warning=255 \
    --diag_wrap=off \
    --gen_func_subsections=on \
    --float_support=vfplib \
    --symdebug:dwarf \
    "-I$(CCS_ARMCOMPILER)/include"

LFLAGS = "-l$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/display/lib/display.aem3" \
    "-l$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/grlib/lib/ccs/m3/grlib.a" \
    "-l$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/drivers/rf/lib/rf_multiMode_cc13x0.aem3" \
    "-l$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/drivers/lib/drivers_cc13x0.aem3" \
    "-l$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/drivers/pdm/lib/pdm_cc13x0.aem3" \
    "-l$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/kernel/tirtos/packages/ti/dpl/lib/dpl_cc13x0.aem3" \
    "-l$(KERNEL_BUILD)/ccs/linker.cmd" \
    "-l$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/devices/cc13x0/driverlib/bin/ccs/driverlib.lib" \
    ../../tirtos/ccs/CC1350_LAUNCHXL_TIRTOS.cmd \
    "-m$(NAME).map" \
    --warn_sections \
    --display_error_number \
    --diag_wrap=off \
    --rom_model \
    "-i$(CCS_ARMCOMPILER)/lib" \
    -llibc.a

all: $(NAME).out

$(CONFIGPKG)/linker.cmd $(CONFIGPKG)/compiler.opt:
	@ $(ECHOBLANKLINE)
	@ echo $(CONFIGPKG) is not built.
	@ echo You can build it by issuing $(MAKE) in $(CONFIGPKG).
	@ $(ECHOBLANKLINE)

EasyLink.obj: ../../easylink/EasyLink.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< --cmd_file=$(CONFIGPKG)/compiler.opt --output_file=$@

AtControl.obj: ../../at/AtControl.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< --cmd_file=$(CONFIGPKG)/compiler.opt --output_file=$@

AtParams.obj: ../../at/AtParams.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< --cmd_file=$(CONFIGPKG)/compiler.opt --output_file=$@

AtProcess.obj: ../../at/AtProcess.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< --cmd_file=$(CONFIGPKG)/compiler.opt --output_file=$@

AtTerm.obj: ../../at/platform/tirtos/AtTerm.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< --cmd_file=$(CONFIGPKG)/compiler.opt --output_file=$@

smartrf_settings.obj: ../../smartrf_settings/smartrf_settings.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< --cmd_file=$(CONFIGPKG)/compiler.opt --output_file=$@

smartrf_settings_predefined.obj: ../../smartrf_settings/smartrf_settings_predefined.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< --cmd_file=$(CONFIGPKG)/compiler.opt --output_file=$@

Per.obj: ../../Per.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< --cmd_file=$(CONFIGPKG)/compiler.opt --output_file=$@

rfEasyLinkNp.obj: ../../rfEasyLinkNp.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< --cmd_file=$(CONFIGPKG)/compiler.opt --output_file=$@

ccfg.obj: ../../ccfg.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< --cmd_file=$(CONFIGPKG)/compiler.opt --output_file=$@

CC1350_LAUNCHXL.obj: ../../CC1350_LAUNCHXL.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< --cmd_file=$(CONFIGPKG)/compiler.opt --output_file=$@

$(NAME).out: $(OBJECTS) $(CONFIGPKG)/linker.cmd
	@ echo linking...
	@ $(LNK) $(CFLAGS) -z $(OBJECTS) $(LFLAGS) -o $(NAME).out

clean:
	@ echo Cleaning...
	@ $(RM) $(OBJECTS) > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).out > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).map > $(DEVNULL) 2>&1
