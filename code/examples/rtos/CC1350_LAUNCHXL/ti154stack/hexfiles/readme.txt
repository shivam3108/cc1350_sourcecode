The TI-15.4-Stack SDK provides Code Composer Studio (CCS) projects that are used
to build applications to run on the CC13xx LaunchPads. The CCS projects produce
two types of output files - .out and .hex, which are used for debugging and
direct downloading, respectively.

This SDK includes a set of pre-built .hex files that provide 'out-of-the-box'
implemetations of the CCS projects, as well as, several specific application
"use cases" for a Sensor and Collector network scenario. These .hex files can
be used to program CC13xx LaunchPad boards via simple device programming tool,
such as SmartRF Flash Programmer 2, without the need to build the code.

Three types of .hex files are included herein:
   - Co-Processor Device
   - Collector Application
   - Sensor Application

The Co-Processor .hex file implements a fully-functional MAC Co-Processor
device, incorporating the TI MAC Stack to support networks up to 50 devices.

The "default", default_433, default_lrm default_palna and "usecase" Collector 
.hex files implement fully-functional network Coordinator devices, 
with Collector application, incorporating the TI MAC Stack to support networks 
up to 50 devices.

The "default" default_433, default_lrm default_palna and "usecase" Sensor .hex 
files implement fully-functional network End-Devices, with Sensor application, 
incorporating the TI MAC Stack to support networks up to 5 devices.

In default folder, here is definition of prefix 
	1. collector_oad_ : colelctor OAD project
	2. sensor_ble_ : sensor with BLE-ADV
	3. sensor_oad_ : sensor with OAD features
	4. sensor_oad_ble: sensor with BLE and OAD features.
	
The folder default_433 contains files for 433Mhz LP.
The folder default_863 contains files for ETSI 863Mhz mode operation.
The folder default_lrm contains files for FCC band LRM mode operation.
The folder default_palna contains file for FCC CC1190 mode.

Application configuration header files (*.h) in the "default" and "usecases"
folders are used to build pairs of Collector and Sensor applications that will
communicate with each other using the same RF and 'network' parameters. Folder
names for the "usecases" reflect the specific communication scenario, as do the
included .h and .hex files. For example:

Consider the files at <as sdk installed>\hexfiles\usecases\nonbeacon_secure:
  The two application header files,
    "collector_cc13xx_lp_pan0xACDC_nonbeacon_secure.h"
    "sensor_cc13xx_lp_pan0xACDC_nonbeacon_secure.h"

  Were used with "out-of-the-box" CCS projects,
    "<as sdk installed>\examples\collector\cc13xx\ccs\.project"
    "<as sdk installed>\examples\sensor\cc13xx\ccs\.project"

  To produce APPLICATION ONLY device .hex files,
    "<as sdk installed>\examples\collector\cc13xx\ccs\collector_cc13xxLP\collector_cc13xx_lp.hex"
    "<as sdk installed>\examples\sensor\cc13xx\ccs\sensor_cc13xxlp\sensor_cc13xx_lp.hex"

  Which were renamed (accrding to usecase) to create the programmable HEX image files,
    "collector_cc13xx_lp_pan0xACDC_nonbeacon_secure.hex"
    "sensor_cc13xx_lp_pan0xACDC_nonbeacon_secure.hex"
