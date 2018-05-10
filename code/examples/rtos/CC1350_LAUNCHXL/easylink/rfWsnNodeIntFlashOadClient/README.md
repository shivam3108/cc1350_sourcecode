## Example Summary

The WSN Node example illustrates how to create a Wireless Sensor Network Node device
which sends packets to a concentrator. This example is meant to be used with the WSN
Concentrator example to form a one-to-many network where the nodes send messages to
the concentrator.

This examples showcases the use of several Tasks, Semaphores, and Events to get sensor
updates and send packets with acknowledgement from the concentrator. For the radio
layer, this example uses the EasyLink API which provides an easy-to-use API for the
most frequently used radio operations.

## Peripherals Exercised

* `Board_PIN_LED0` - Toggled when the a packet is sent
* `Board_ADCCHANNEL_A0` - Used to measure the Analog Light Sensor by the SCE task
* `Board_PIN_BUTTON0` - Selects fast report or slow report mode. In slow report
mode the sensor data is sent every 5s or as fast as every 1s if there is a
significant change in the ADC reading. The fast reporting mode sends the sensor data
every 1s regardless of the change in ADC value. The default is slow
reporting mode.


## Resources & Jumper Settings

> If you're using an IDE (such as CCS or IAR), please refer to Board.html in your
project directory for resources used and board-specific jumper settings. Otherwise, you
can find Board.html in the directory &lt;SDK_INSTALL_DIR&gt;/source/ti/boards/&lt;BOARD&gt;.

## Example Usage

* Run the example. On another board run the WSN Concentrator example. This node should
show up on the LCD of the Concentrator.

The example also supports Over The Air Download (OAD), where new FW can be transferred
from the concentrator to the node. There must be an OAD Server, which is included in
the concentrator project, and an OAD client, which is included in the node  project.

### Performing an OAD Image Transfer

To be safe the external flash of the Concentrator should be wiped before running the
example. To do this, program both LP boards with erase_extflash_cc13x0lp.hex. The
program will flash the LEDs while erasing the external flash. Allow the application to
run until the LEDs stop flashing indicating the external flash has been erased.

The FW to erase the external flash can be found in below location and should be loaded
using Uniflash programmer:
`<SDK_DIR>/examples/rtos/CC1310_LAUNCHXL/easylink/hexfiles/onChipOad/ccs/erase_extflash_cc13x0lp.hex`

In addition, the internal flash of the node should be wiped to ensure no corrupt
software already resides on the node. This can be done through the Settings and
Utilities tab in Uniflash.

The Concentrator OAD Server and Node OAD Client FW should each be loaded into a 
CC1310LP/CC1350LP using the Uniflash programmer:

- Load rfWsnConcentratorOadServer (.out) project into a CC1310LP/CC1350LP
- Load `<SDK_DIR>/examples/rtos/CC1310_LAUNCHXL/easylink/hexfiles/onChipOad/ccs/rfWsnNodeIntFlashOadClient_CC1310_LAUNCHXL_all_v1.bin` into
a CC1310LP/CC1350LP

The Concentrator will display the below on the UART terminal:

```shell
Nodes   Value   SW    RSSI
*0x0b    0887    0    -080
 0xdb    1036    0    -079
 0x91    0940    0    -079
Action: Update available FW
Info: Available FW unknown
```

Use the node display to identify the corresponding node ID:

```shell
Node ID: 0x91
Node ADC Reading: 1196
```

The node OAD image can be loaded into the external flash of the Concentrator through
the UART with the oad_write_bin.py script. The action must first be selected using 
BTN-2. Press BTN-2 until the Action is set to `Update available FW`, then press BTN-1
and BTN-2 simultaneously to execute the action.

When "Available FW" is selected the terminal will display:

```shell
    Waiting for Node FW update...
```

The UART terminal must be closed to free the COM port before the script is run. Then
the python script can be run using the following command:

```shell
    python <SDK>/tools/easylink/oad/oad_write_bin.py /dev/ttyS28 <SDK_DIR>/examples/rtos/CC1310_LAUNCHXL/easylink/hexfiles/onChipOad/ccs/rfWsnNodeIntFlashOadClient_CC1310_LAUNCHXL_app_v2.bin
```

After the download the UART terminal can be re-opened and the "Info" menu line will be
updated to reflect the new FW available for OAD to a node.

The current FW version running on the node can be requested using the `Send FW Ver Req`
action. This is done by pressing BTN-1 until the desired node is selected (indicated by
the \*), then pressing BTN-2 until the Action is set to `Send FW Ver Req`. To execute the
action, press BTN-1 and BTN-2 simultaneously.

The next time the node sends data to the concentrator, the FW version of the selected
node will appear in the Info section.

```shell
Nodes   Value   SW    RSSI
 0x0b    0887    0    -080
 0xdb    1036    0    -079
*0x91    0940    0    -079
Action: Send FW Ver Req
Info: Node 0x91 FW v1.0
```

The node FW can now be updated to the image stored on the external flash of the
concentrator. Press BTN-1 until the desired node is selected, then press BTN-2 until
the Action is set to `Update node FW`. To execute the action, press BTN-1 and BTN-2 
simultaneously.

The next time the node sends data, the OAD sequence will begin. As the node requests
each image block from the concentrator the Concentrator display is updated to show the 
progress of the image transfer.

```shell
Nodes   Value   SW    RSSI
 0x0b    0887    0    -080
 0xdb    1036    0    -079
*0x91    0940    0    -079
Action: Update node FW
Info: OAD Block 14 of 1089
```

The node display also updates to show the status of the image transfer.
```shell
Node ID: 0x91
Node ADC Reading: 3093
OAD Block: 14 of 1089
OAD Block Retries: 0
```


Once the OAD has completed, the concentrator will indicate that the transfer has
finished with an `OAD Complete` status. The node will reset itself with a new node ID.
If the device does not reset itself a manual reset may be necessary.

### Generating OAD Images

For generating the images the following tools are required:
- `Code Composer Studio`
-- Download the latest version from [http://processors.wiki.ti.com/index.php/Download_CCS]( http://www.ti.com/tool/CCSTUDIO)
- `Simplelink CC13X0 SDK`
-- Download the latest version from [http://www.ti.com/tool/simplelink-cc13x0-sdk](http://www.ti.com/tool/simplelink-cc13x0-sdk)
- `Python 2.7`
- `Python intelhex-2.1`
- `Python crcmod-1.7`

Onchip OAD partions the available flash into two regions denoted as Image A and Image B.
The image transferred using OAD should correspond to the image region where
the application is not running. For example, if Image A is loaded on the LP, then
the OAD should be performed with an Image B binary. The following steps walk through
how to create an application binary, a production binary, and how to modify the
project to create either Image A or Image B.

#### Creating the BIM Image 
The BIM is generated from the SDK project:
`<SDK_DIR>/examples/rtos/CC1310_LAUNCHXL/easylink/bim_onchip/tirtos/ccs/bim_intflash_cc13x0lp_app.projectspec`
A prebuilt hex image is located in the SDK:
`<SDK_DIR>/examples/rtos/CC1310_LAUNCHXL/easylink/hexfiles/onChipOad/ccs/bim_intflash_cc13x0lp.hex`

#### Creating the Application Image

To generate the application hex file (Image A), the project should be imported and built with the
desired compiler. To change the FW version, update the following string in `native_oad/oad_client.c`
```shell
#define FW_VERSION "v1.0"
```

The OAD binary can be created from the application hex file using the OAD image tool found in the following location: 
`<SDK_DIR>/tools/easylink/oad/oad_image_tool.py`

To use the OAD image tool execute the following command:
```shell
python oad_image_tool.py -t onchip -i app -v 0x0100 -m 0x1000 -ob rfWsnNodeIntFlashOadClient_CC1310_LAUNCHXL_app_v1.bin rfWsnNodeIntFlashOadClient_CC1310_LAUNCHXL_app_v1.hex
```

The following command-line arguments can be must be used to build the image:
* `-t onchip` selects the type of OAD between onchip and offchip
* `-i app` is used to indicate the BIM is not in included, use production when the hex file also contains the BIM 
* `-m 0x1000` indicates the start of the image, use address `-m 0x10000` for Image B
* `-v 0x0100` refers to the FW version with the format 0xXXYY where XX is the major version and YY the minor version (for example 2.3 would be 0x0203).
**Note: The FW Version is defined in native_oad/oad_client.c** 

#### Modifying the Project to Create Image B

The linker and compiler flags need to be updated from `HAL_IMAGE_A` to `HAL_IMAGE_B`
to create the Image B application hex. 

To update these in Code Composer Studio, navigate to Project &rarr; Properties &rarr; Build. From the Arm Compiler category update the flag set by selecting Edit Flags. Update the `--define=HAL_IMAGE_A` to `--define=HAL_IMAGE_B`. Update the linker flags by making the same modification from the ARM Linker category.

To update the compiler and linker flags in IAR Embedded Workbench, navigate to Options &rarr; C/C++ Compiler. From the Extra Options tab, update `-DHAL_IMAGE_A` to `-DHAL_IMAGE_B`. Then from the Extra Options tab under Options &rarr; Linker, update `--config_def=HAL_IMAGE_A=1` to `--config_def=HAL_IMAGE_B=1`. 

**Make sure to clean the project before building to update the linker.cmd file with
the new settings**

Build the project to create the application hex file for Image B.
To create the application binary, use the OAD image tool and modify the -m flag as 
shown below.
```shell
python oad_image_tool.py -t onchip -i app -v 0x0200 -m 0x10000 -ob rfWsnNodeIntFlashOadClient_CC1310_LAUNCHXL_app_v2.bin rfWsnNodeIntFlashOadClient_CC1310_LAUNCHXL_app_v2.hex
```

#### Creating the Production Image

The production image (binary) is the image that should be flashed directly to the LP.

First, the application image (hex) is the merged with the BIM (hex) to create the production image (hex).
```shell
python /usr/bin/hexmerge.py -o rfWsnNodeIntFlashOadClient_CC1310_LAUNCHXL_all_v1.hex 
"--overlap=error" rfWsnNodeIntFlashOadClient_CC1310_LAUNCHXL_app_v1.hex
bim_intflash_cc13x0lp.hex
```

The oad_image_tool creates an OAD image header that contains information used by the
application and the BIM. Therefore, the oad_image_tool is used to generate an OAD 
production binary that contains the onchip BIM, OAD image header, and application for 
flashing to the device.

```shell
python oad_image_tool.py -t onchip -i production -v 0x0100 -m 0x1000 -ob
rfWsnNodeIntFlashOadClient_CC1310_LAUNCHXL_all_v1.bin 
rfWsnNodeIntFlashOadClient_CC1310_LAUNCHXL_all_v1.hex
```
The production image type (-i) is used so the oad_image_tool can accurately calculate the CRC of the image.

## Application Design Details

* This examples consists of two tasks, one application task and one radio
protocol task. It also consists of a Sensor Controller Engine (SCE) Task which
samples the ADC.

* On initialization the CM3 application sets the minimum report interval and
the minimum change value which is used by the SCE task to wake up the CM3. The
ADC task on the SCE checks the ADC value once per second. If the ADC value has
changed by the minimum change amount since the last time it notified the CM3,
it wakes it up again. If the change is less than the masked value, then it
does not wake up the CM3 unless the minimum report interval time has expired.

* The NodeTask waits to be woken up by the SCE. When it wakes up it toggles
`Board_PIN_LED1` and sends the new ADC value to the NodeRadioTask.

* The NodeRadioTask handles the radio protocol. This sets up the EasyLink
API and uses it to send new ADC values to the concentrator. After each sent
packet it waits for an ACK packet back. If it does not get one, then it retries
three times. If it did not receive an ACK by then, then it gives up.


* *RadioProtocol.h* can also be used to configure the PHY settings from the following
options: IEEE 802.15.4g 50kbit (default), Long Range Mode or custom settings. In the
case of custom settings, the *smartrf_settings.c* file is used. The configuration can
be changed by exporting a new smartrf_settings.c file from Smart RF Studio or
modifying the file directly.

## References
* For more information on the EasyLink API and usage refer to [SimpleLink-EasyLink](http://processors.wiki.ti.com/index.php/SimpleLink-EasyLink).
