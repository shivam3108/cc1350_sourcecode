## Example Summary

The WSN Concentrator example illustrates how to create a simple Wireless Sensor
Network Concentrator device which listens for packets from other nodes. This
example is meant to be used together with the WSN Node example to form a one-
to-many network where the nodes send messages to the concentrator.

This examples showcases the use of several Tasks, Semaphores and Events to
receive packets, send acknowledgements and display the received data on the
LCD. For the radio layer, this example uses the EasyLink API which provides
an easy-to-use API for the most frequently used radio operations.

Refer to [EasyLink API](http://processors.wiki.ti.com/index.php/SimpleLink-EasyLink)
for more information on the EasyLink API.


## Peripherals Exercised
* `Board_PIN_LED0` - Toggled when data is received over the RF interface
* `Board_PIN_BUTTON0` - Select Node
* `Board_PIN_BUTTON1` - Select Action

## Resources & Jumper Settings

> If you're using an IDE (such as CCS or IAR), please refer to Board.html in your project
directory for resources used and board-specific jumper settings. Otherwise, you can find
Board.html in the directory &lt;SDK_INSTALL_DIR&gt;/source/ti/boards/&lt;BOARD&gt;.

## Example Usage
Run the example. On another board (or several boards) run the WSN Node example.
The LCD will show the discovered node(s). When the collector receives data from
a new node, it is given a new row on the display and the received value is shown.
If more than 7 nodes are detected, the device list rolls over, overriding
the first. Whenever an updated value is received from a node, it is updated on
the LCD display.

The example also supports Over The Air Update (OAD), where new FW can be loaded over OAD.
The must be an OAD Server, which is included in the concentrator, and an OAD client which
is included in the sensor.

** See rfWsnNode(Int/Ext)FlashOadClient project README for instructions on how to generate OAD images **

### Performing an OAD Image Transfer

To be safe the external flash of the Concentrator should be wiped before running the
example. To do this, program both LP boards with erase_extflash_cc13x0lp.hex. The
program will flash the LEDs while erasing the external flash. Allow the application to
run until the LEDs stop flashing indicating the external flash has been erased.

The FW to erase the external flash can be found in below location and should be loaded
using Uniflash programmer:
`<SDK_DIR>/examples/rtos/CC1310_LAUNCHXL/easylink/hexfiles/offChipOad/ccs/erase_extflash_cc13x0lp.hex`

The Concentrator OAD Server and Node OAD Client FW should each be loaded into a 
CC1310LP/CC1350LP using the Uniflash programmer:

- Load rfWsnConcentratorOadServer (.out) project into a CC1310LP/CC1350LP
- Load `<SDK_DIR>/examples/rtos/CC1310_LAUNCHXL/easylink/hexfiles/offChipOad/ccs/rfWsnNodeExtFlashOadClient_CC1310_LAUNCHXL_all_v1.bin` into
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
    python <SDK>/tools/easylink/oad/oad_write_bin.py /dev/ttyS28 <SDK_DIR>/examples/rtos/CC1310_LAUNCHXL/easylink/hexfiles/offChipOad/ccs/rfWsnNodeExtFlashOadClient_CC1310_LAUNCHXL_app_v2.bin
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

## Application Design Details

This examples consists of two tasks, one application task and one radio
protocol task.

The ConcentratorRadioTask handles the radio protocol. This sets up the EasyLink
API and uses it to always wait for packets on a set frequency. When it receives
a valid packet, it sends an ACK and then forwards it to the ConcentratorTask.

The ConcentratorTask receives packets from the ConcentratorRadioTask, displays
the data on the LCD and toggles Board_PIN_LED0.

*RadioProtocol.h* can also be used to change the
PHY settings to be either the default IEEE 802.15.4g 50kbit,
Long Range Mode or custom settings. In the case of custom settings,
the *smartrf_settings.c* file is used. This can be changed either
by exporting from Smart RF Studio or directly in the file.

Note for IAR users: When using the CC1310DK, the TI XDS110v3 USB Emulator must
be selected. For the CC1310_LAUNCHXL, select TI XDS110 Emulator. In both cases,
select the cJTAG interface.

## References
* For more information on the EasyLink API and usage refer to [SimpleLink-EasyLink](http://processors.wiki.ti.com/index.php/SimpleLink-EasyLink).
