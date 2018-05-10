SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR ?= $(abspath ../../../../../../..)

include $(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/imports.mak

XDCPATH = $(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source;$(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/kernel/tirtos/packages;

CC = "$(CCS_ARMCOMPILER)/bin/armcl"
LNK = "$(CCS_ARMCOMPILER)/bin/armcl"

XDCTARGET = ti.targets.arm.elf.M3
PLATFORM = ti.platforms.simplelink:CC1350F128

OBJECTS = swi.obj CC1350_LAUNCHXL.obj ccfg.obj

CONFIGPKG = swi

NAME = swi

CFLAGS = -I../.. \
    -DDeviceFamily_CC13X0 \
    "-I$(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source" \
    "-I$(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/posix/ccs" \
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

LFLAGS = "-i$(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source" \
    -lti/display/lib/display.aem3 \
    -lti/grlib/lib/ccs/m3/grlib.a \
    -lthird_party/spiffs/lib/ccs/m3/spiffs_cc26xx.a \
    -lti/drivers/rf/lib/rf_multiMode_cc13x0.aem3 \
    -lti/drivers/lib/drivers_cc13x0.aem3 \
    -lti/drivers/pdm/lib/pdm_cc13x0.aem3 \
    "-i$(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/kernel/tirtos/packages" \
    -lti/dpl/lib/dpl_cc13x0.aem3 \
    -lti/devices/cc13x0/driverlib/bin/ccs/driverlib.lib \
    ../../tirtos/ccs/CC1350_LAUNCHXL_TIRTOS.cmd \
    "-m$(NAME).map" \
    --warn_sections \
    --display_error_number \
    --diag_wrap=off \
    --rom_model \
    "-i$(CCS_ARMCOMPILER)/lib" \
    -llibc.a

.PRECIOUS: $(CONFIGPKG)/compiler.opt $(CONFIGPKG)/linker.cmd

all: $(NAME).out

$(CONFIGPKG)/compiler.opt: $(CONFIGPKG)/linker.cmd

$(CONFIGPKG)/linker.cmd: ../../tirtos/swi.cfg
	@ echo Running Configuro...
	@ $(XDC_INSTALL_DIR)/xs --xdcpath="$(XDCPATH)" xdc.tools.configuro -c "$(CCS_ARMCOMPILER)" -t $(XDCTARGET) -p $(PLATFORM) --compileOptions "$(CFLAGS)" -o "$(CONFIGPKG)" ../../tirtos/swi.cfg

swi.obj: ../../swi.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< --cmd_file=$(CONFIGPKG)/compiler.opt --output_file=$@

CC1350_LAUNCHXL.obj: ../../CC1350_LAUNCHXL.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< --cmd_file=$(CONFIGPKG)/compiler.opt --output_file=$@

ccfg.obj: ../../ccfg.c $(CONFIGPKG)/compiler.opt
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< --cmd_file=$(CONFIGPKG)/compiler.opt --output_file=$@

$(NAME).out: $(OBJECTS) $(CONFIGPKG)/linker.cmd
	@ echo linking...
	@ $(LNK) $(CFLAGS) -z $(OBJECTS) -l$(CONFIGPKG)/linker.cmd $(LFLAGS) -o $(NAME).out

clean:
	@ echo Cleaning...
	@ $(RM) $(OBJECTS) > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).out > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).map > $(DEVNULL) 2>&1
	@ $(RMDIR) $(NAME) > $(DEVNULL) 2>&1
