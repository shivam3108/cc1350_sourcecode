15.4 OAD Example
===================================

This example contains the
sensor_oad and project intended to be used with either the Linux collector for
updating the FW over the 15.4 network or over a BLE link using an OAD capable
BLE central such as the TI BLE Device Monitor. For OAD using the BLE link the
CC1350 OAD Client must first be running the BLE Simple Peripheral project.

**If using a CC1310, only 15.4 native OAD is supported.**

**To save space for features such as `FEATURE_NATIVE_OAD` and `FEATURE_ALL_MODES` this project  builds with TIRTOS_IN_ROM by default. This will produce an image that is incapable of performing BLE_OAD. To learn more see the "ROM'ed TIRTOS Optimizations" section below **

**Due to flash limitations, when `FEATURE_NATIVE_OAD` is defined it may be necessary to only define a single transmission mode in order for the project to have enough flash space to build properly. (i.e FEATURE_FREQ_HOP_MODE , FEATURE_BEACON_MODE )**

Generating the required binary images
=====================================

For generating the images the following tools are required:

- `CCSv7` [http://processors.wiki.ti.com/index.php/Download_CCS](http://processors.wiki.ti.com/index.php/Download_CCS)
- `simplelink_cc13x0_sdk_1_40_00_xx` [http://www.ti.com/tool/simplelink-cc13x0-sdk](http://www.ti.com/tool/simplelink-cc13x0-sdk)
- `Python 2.7`
- `Python intelhex-2.1`
- `Python crcmod-1.7`
- `Uniflash` [http://www.ti.com/tool/uniflash](http://www.ti.com/tool/uniflash)

Only the OAD client (sensor_oad) requires the BIM. The prebuilt BIM binary is generated
from the BLE Stack project:

* CCS:
 `<SDK_DIR>\examples\rtos\CC1350_LAUNCHXL\blestack\util\bim_extflash\tirtos\ccs`
* IAR: `<SDK_DIR>\examples\rtos\CC1350_LAUNCHXL\blestack\util\bim_extflash\tirtos\iar`

The python intelhex merge utility is used to combine the BIM and 15.4 Stack App into one
hex file that can be downloaded with SmartRF Studio:

```shell
cd <SDK_DIR>/examples/rtos/CC1310_LAUNCHXL/ti154stack/hexfiles/oad
python /usr/bin/hexmerge.py -o sensor_oad_cc13x0lp_all.hex "--overlap=error" sensor_oad_cc13x0lp.hex bim_extflash_cc1350lp.hex
```

The 15.4 Stack OAD image can be created from an application image with the oad_image_tool.py:
**Note: By default, this project builds with `TIRTOS_IN_ROM` which means the metadata location flag is `-m 0x14F0`. However, if you change the project to build with TIRTOS in Flash you should use a metadata location flag of `-m 0x1000`.**

```shell
python  ../../../../../../tools/common/oad/oad_image_tool.py -v [0xXXYY] -i app sensor_oad_cc13x0lp.hex -ob sensor_oad_cc13x0lp_app.bin -m 0x14F0 -r :0x1E000
```

Where XX is the major version and YY is the minor version

The `oad_image_tool.py` can be used to create an OAD binary that erases NV during download, and hence causes the sensor to join a new 15.4 network after OAD.

For 15.4 OAD where the device forgets the network the image should be crated without the `-r :0x1E000` option:

```shell
python ../../../../../../tools/common/oad/oad_image_tool.py -v [0xXXYY] -i app sensor_oad_cc13x0lp.hex -ob sensor_oad_cc13x0lp_app-nverase.bin -m 0x14F0
```.

To create the combnined BLE App, Stack and BIM image used for flashing the Luanchpad, use oad_image_tool:

```shell
python /../../../../../../tools/common/oad/oad_image_tool.py -i production -t offchip -o simple_peripheral_cc1350lp_all.hex bim_extflash_cc1350lp.hex simple_peripheral_cc1350lp_app.hex simple_peripheral_cc1350lp_stack.hex 
```

The `oad_image_tool.py` can be used to create an BLE OAD binary containing the BLE App and Stack:

```shell
python ../../../../../../tools/common/oad/oad_image_tool.py -v [0xXXYY] -i stack simple_peripheral_cc1350lp_app.hex simple_peripheral_cc1350lp_stack.hex -ob simple_peripheral_cc1350lp_app_stack.bin -m 0x1000
```

---
**Note: on versioning**

To give a different version string the ```FW_VERSION``` define must be updated in oad_client.c and the -v [0xXXYY], bust be changed to reflect this when building the OAD bin. For example:

```code
#define FW_VERSION "v10.12"
```

Would need an oad_image_tool -v option of:

```shell
-v 0x1012
```
---

Using the OAD example for native 15.4 OAD
=====================

To be safe when using for the first time the external flash of the sensor should be wiped. Program both LP boards with
`CC1350LaunchPad_ExtFlashErase.hex`. The program will flash the LED's while erasing and once finished the LED's will stop flashing.
Allow the application to run until the external flash has been erased and the LED's stop flashing.

The wipe flash FW can be found in below location and should be downloaded with uniflash programmer if external Flash need to be erased:

`<SDK_DIR>/examples/rtos/CC1310_LAUNCHXL/ti154stack/hexfiles/native_oad/CC1350LaunchPad_ExtFlashErase.hex`


If not already done so, the 15.4Mac Co Processor and Sensor OAD Client FW must then be loaded in to the
LP's. This can be done using the uniflash programmer:

- Load `<SDK_DIR>/examples/rtos/CC1310_LAUNCHXL/ti154stack/hexfiles/coprocessor_cc13xx_lp.hex` into a
CC1350LP
- Load `<SDK_DIR>/examples/rtos/CC1310_LAUNCHXL/ti154stack/hexfiles/native_oad/sensor_oad_cc13x0lp_all.hex` into
a CC1310LP

The Linux Collector must then be run. More information can be found on using the Linux collector in the Linux SDK Documentation, to use the
Linux collector for OAD make sure it is built without the `IS_HEADLESS` pre-defined symbol. When ran the Linux Collector will display the following
UI on the terminal:

```shell
TI Collector
Nwk: Started
Sensor 0x0001: Temp 25, RSSI -18
Sensor 0x0002: Temp 22, RSSI -32


Info: ConfigRsp 0x0002
cmd:
```

The available commands are:

```shell
sxx: Select a device. Example 's1'| 's0x1234'
o:   Toggle the permit join
t:   Send an LED toggle request to selected device
v:   Send a version request to selected device
u:   Send FW update request to selected device
d:   Send disassociation request to selected device
fxx: Set FW file from configured OAD FW dir. Example 'f sensor_mac_oad_cc13x0lp_app.bin'
```

Once joined to the network the sensor must be selected with the command `s1`. This will display:

```shell
TI Collector
Nwk: Started
Sensor 0x0001: Temp 25, RSSI -18
Sensor 0x0002: Temp 22, RSSI -32


Info: Selected device 0x0001
cmd: s1
```

The current FW version running on the sensor can be requested using the `v` command. The next time the sensor polls it will respond and the FW version will be displayed:

```shell
TI Collector
Nwk: Started
Sensor 0x0001: FW Ver v1.0
Sensor 0x0002: Temp 22, RSSI -32


Info: Sending 0x0001 FW Version Req
cmd: v
```

The FW image file to update can be selected using the `f <path_to_File/file_name.bin>` command:

```shell
TI Collector
Nwk: Started
Sensor 0x0001: FW Ver v1.0
Sensor 0x0002: Temp 22, RSSI -32


Info: OAD file ../../firmware/oad/sensor_oad_cc13x0lp_app.bin
cmd: f ../../firmware/oad/sensor_oad_cc13x0lp_app.bin
```

Entering just `f` will report the currently selected file.

A FW update of the selected sensor can be initiate with the `u` command. The next time the sensor polls it will process the request and if the FW update is accepted the
sensor will start requesting OAD image blocks:

```shell
TI Collector
Nwk: Started
Sensor 0x0001: OAD Block 41 of 928
Sensor 0x0002: Temp 22, RSSI -32


Info: Sending 0x0001 FW Update Req
cmd: u
```

Depending on the build options, the sensor may also display the block request on the serial port and/or LCD:

```shell
OAD Block: 1
of 928
OAD Block: 2
of 928
```

Once OAD is complete the sensor will reboot and depending on if the NV page was included in the image the sensor may join the network.

```shell
OAD Block: 927
of 928
OAD completed successfully
TI Sensor
State Changed: 2
Restarted: 0x1
Channel: 0
State Changed: 4
```

The Linux collector can then verify that the new FW version is running on the sensor with the `v` command:

```shell
TI Collector
Nwk: Started
Sensor 0x0001: FW Ver v2.0
Sensor 0x0002: Temp 22, RSSI -32


Info: Sending 0x0001 FW Version Req
cmd: v
```

Native 15.4 OAD Pause and Resume
=====================
The native OAD feature supports the following robustness features:
1. Timeouts: This is when a the data request for a block response is not answered. The data request delay from an OAD block request being sent is set by OAD_BLOCK_REQ_POLL_DELAY ms, it is advised that this be set as short as possible to avoid unnecessary queueing of data in the Co-Processor. A timeout is typically caused by the Linux Collector taking too long to read the OAD Block from the FW Image file (due to CPU load). The number of timeouts before a retry is set by OAD_MAX_TIMEOUTS.
2. Retries: A retry is when the maximum number timeouts has expired before the OAD Block Response has been received. In his case the OAD Block Request is resent. The number of retires before an OAD Abort is set by OAD_MAX_RETRIES.
3. Aborts: The OAD is aborted after there are OAD_MAX_RETRIES block requests with no response. After an OAD abort the OAD is attempted to be resumed after OAD_BLOCK_AUTO_RESUME_DELAY ms, if the OAD abort again on the same block number the OAD is terminated.

The OAD is aborted if the device Orphans, when the device rejoins an OAD resume is attempted.

If the device is reset / powered off during an OAD the device will attempt to resume when it rejoins. The block it resumes from is set to the first block of the page it was aborted from in case the flash page was corrupted by the power cycle.


Support for multiple OAD files
=====================
The OAD protocol supports multiple OAD images by using an Image ID that is sent when the Collector initiates the OAD and then in each OAD block request / response. This insures that the device always receives a block from the correct FW image, especially in the case where a device loses power or orphans and it is not known when it will come back on line. When an OAD image file is selected on the collector it is assigned a new image ID and added to a table, when a block request is received the image ID in the block request is used to find the correct FW image file. This insures that a device will always get a block from the correct image, no matter how long it is off line.

Native 15.4 OAD parameters and default settings
=====================

Most OAD settings are defined in oad_cleint.c, and already discussed in the Pause and Resume section. In addition to this you can override the OAD_BLOCK_SIZE from the default of 128 by defining it in the project options. Setting this higher than 128 is not advised as this is the setting used during system testing. 

Beacon Mode, Non Beacon Mode and Frequency Hoping network modes are supported. In Non Beacon Mode and Frequency Hoping the default OAD parameters are:

```c
#define OAD_BLOCK_REQ_RATE            200
#define OAD_BLOCK_REQ_POLL_DELAY      40
#define OAD_MAX_TIMEOUTS              3
#define OAD_MAX_RETRIES               3
#define OAD_BLOCK_AUTO_RESUME_DELAY   5000
```

Under normal conditions the sensor sends a block request every 200ms, with a typical file of 920 blocks this takes ~3 minutes.

In Beacon Mode only one data request can be sent during 1 beacon interval. The default OAD settings are:

```c
#define BEACON_INTERVAL             ((((0x01) << (CONFIG_BEACON_ORDER)) * (SYMBOL_DURATION) * (BASE_SUPER_FRAME_DURATION))/ (1000)) // ms

#define OAD_BLOCK_REQ_RATE          ((BEACON_INTERVAL) - 100)
#define OAD_BLOCK_REQ_POLL_DELAY    ((BEACON_INTERVAL) - 400)
#define OAD_MAX_TIMEOUTS            3
#define OAD_MAX_RETRIES             3
#define OAD_BLOCK_AUTO_RESUME_DELAY ((BEACON_INTERVAL) * 5)
```

BEACON_INTERVAL is calculated with this equation `BEACON_INTERVAL = 2 ^ (CONFIG_BEACON_ORDER) * 960 * SYMBOL_DURATION_IN_MS`
This means that while in Beacon Mode there are two things that will effect the speed of your OAD transfer. The `CONFIG_BEACON_ORDER` you choose and the PHY_ID that you choose.

Additionally the constant values (100 and 400) being subtracted from the `REQ_RATE` and `REQ_POLL_DELAY` can be modified to add slightly better performance

With a 50kbps PHY_ID and a `CONFIG_BEACON_ORDER` of 6 you get a `BEACON_INTERVAL` of 1.228s and an OAD_TRANSFER time of ~10 min
With a 200kbps PHY_ID and a `CONFIG_BEACON_ORDER` of 6 you get a `BEACON_INTERVAL` of  0.307s and an OAD_TRANSFER time of ~2.5 min

It is suggested to start with a the following values:
50kbps  ==> `CONFIG_BEACON_ORDER` = 4
200kbps ==> `CONFIG_BEACON_ORDER` = 6

The sensor_oad project has 2 defines related to te OAD feature, defined by default in the project options:

* `FEATURE_OAD`: Required for BLE OAD. This includes the OAD linker command file and oad TIRTOS config file. Defining this results in the application being placed at location 0x1010, leaving space for the BIM and BIM Header.
* `FEATURE_NATIVE_OAD`: Required for 15.4 native OAD. This includes the 15.4 OAD client, the OAD linker command file and oad TIRTOS config file. This results in an application that supports the OAD messages needed to receive an OAD update over the 15.4 network, as well as the BIM header as described above.

ROM'ed TIRTOS Optimizations
=====================

~8K of code size savings is gained by running TIRTOS in ROM. BLE OAD servers (Device Monitor and the Smart Phone App) require the BIM header to be located at address 0x1000 of internal flash. This clashes with the TIRTOS constant data used when TIRTOS runs from ROM. Therefore, since TIRTOS_IN_ROM is enabled by defualt, compatibility with the BLE and EasyLink OAD is lost and the generated binary can only be supported with 15.4 native OAD using the Linux Collector as the OAD server. Due to flash limitations TIRTOS_IN_ROM has been enabled by default. Because of this reason BLE OAD is not supported without building tirtos in flash.
To disable TIRTOS_IN_ROM and to run TIRTOS in Flash make the following changes.

* Modify the BIM `<SDK_DIR>\examples\rtos\CC1350_LAUNCHXL\blestack\util\bim_extflash\tirtos\ccs`. In Project-> Properties -> Arm Compiler -> Predefined Symbols, remove `TIRTOS_IN_ROM`
* Modify sensor_oad application:
  * In Project-> Properties -> Arm Compiler -> Predefined Symbols, remove `TIRTOS_IN_ROM`
  * In Project-> Properties -> Arm Linker -> Advanced Options -> Command File Processing, remove `TIRTOS_IN_ROM`
  * In the project's configuration file, Tools->app.cfg, comment out the following line 'ROM.romName = ROM.CC1350;' in order use the TIRTOS kernel in Flash.
  * Rebuild the project and create v1 and v2 hex files
  * Recreate the merged hex:

```shell
cd <SDK_DIR>/examples/rtos/CC1310_LAUNCHXL/ti154stack/hexfiles/oad
python /usr/bin/hexmerge.py -o sensor_oad_cc13x0lp_all_v1.hex "--overlap=error" sensor_oad_cc13x0lp_app_v1.hex bim_extflash_cc1350lp.hex
```

  * Recreate the OAD binary, with the meta data located at 0x1000:

```shell
python  ../../../../../../tools/common/oad/oad_image_tool.py -v 0x0200 -i app sensor_oad_cc13x0lp_app_v2.hex -ob sensor_oad_cc13x0lp_app_v2.bin -m 0x1000 -r :0x1E000
```

  * Copy the new sensor_oad_cc13x0lp_app_v1.hex and sensor_oad_cc13x0lp_app_v2.bin to te Linux Collector

* Modify the Linux collector
  * In /linux/example/collector/Makefile, comment out the line `CFLAGS += -DTIRTOS_IN_ROM`.
  * Rebuild the Linux Collector via the `make` command from within the /linux/example/collector directory.

Using the OAD example for BLE 15.4 OAD
=====================

For pushing a BLE OAD you will need to use the BLE device monitor. The BLE device monitor can be downloaded from:
[http://processors.wiki.ti.com/index.php/BLE_Device_Monitor_User_Guide](http://processors.wiki.ti.com/index.php/BLE_Device_Monitor_User_Guide)

The LP attached to device monitor will be the downloader.

The BLE Host Test and BLE Simple Peripheral FW must then be loaded in to the
LP's. This can be done using the uniflash programmer:

- Load `<SDK_DIR>/examples/rtos/CC1310_LAUNCHXL/ti154stack/hexfiles/oad/simple_peripheral_cc1350lp_all.hex` into a
CC1350LP
-Load `<SDK_DIR>/examples/rtos/CC1310_LAUNCHXL/ti154stack/hexfiles/oad/host_test_cc1350lp_all.hex` into a
CC1260LP or CC1350LP

Using BLE Device Monitor download the 15.4Stack FW:

- Connect device monitor to the com port of the LP running the host test FW.
- Reset the LP running the simple_peripheral_cc1350lp_all FW, device monitor should see the BLE advertisement "Simple BLE Peripheral". Establish a BLE connection with the device from the monitor. You should see OAD as one of the services supported by the BLE device.
- Go to file and choose OAD. Then set image type to 2 i.e. stack and load `<SDK_DIR>/examples/rtos/CC1310_LAUNCHXL/ti154stack/hexfiles/ble_oad/simple_peripheral_cc1350lp_no_bim.hex`. This image has the OAD capable BLE stack + app without BIM.
- Start and complete the OAD.
- Wait for device to reset and for BLE advertisement "Simple BLE Peripheral" to show up on the device monitor. Now the device is capable of performing OAD of any image.
- Use OAD to load `<SDK_DIR>/examples/rtos/CC1310_LAUNCHXL/ti154stack/hexfiles/oad/sensor_oad_cc13x0lp_app.hex`.
- Wait for OAD to complete and the device to reset. The BLE advertisement from the device will no longer be observed the monitor. The 15.4 stack image should now be running on the device.
- To switch back to OAD capable BLE, press both left and right buttons at the same time.

---
**Note: on using BLE device monitor**

There are many parameters that can be used to control the BLE connection used for OAD. They are Blocks/connection interval, fast mode will in addition enable various connection intervals.
These parameters will need to be selected as per the deployment requirements.
The slowest parameter setting is the most reliable. Ensure fast mode is not selected and that the Blocks/connection interval is set to one.

---

