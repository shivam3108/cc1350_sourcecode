These prebuilt images are to be used with the native and ble OAD feature.

hexfiles/oad/
├── bim_extflash_cc1350lp.hex - Boot Image Manager hex file
├── CC1350LaunchPad_ExtFlashErase.hex - Utility application to erase external flash
├── empty_app.bin - Small OAD bin for debug and development (does not contain bim). The example will simply boot and tggle the LED's, no 15.4 functionality is included
├── host_test_cc1350lp_all.hex - BLE Host test used as OAD server with BLE Device Monitor
├── sensor_oad_cc13x0lp_all.hex - 15.4 OAD hex that contains contains bim. Intended for download with Uniflash.
├── sensor_oad_cc13x0lp_app.bin - 15.4 OAD bin. Intended for OAD.
├── sensor_oad_cc13x0lp_app.hex - 15.4 OAD hex (does not contain bim). Intended for OAD download with Device Monitor.
├── simple_peripheral_cc1350lp_all.hex - BLE Simple Peripheal containing Application, Stack and BIM. Intended for download with Uniflash.
├── simple_peripheral_cc1350lp_app_stack.bin - BLE Simple Peripheal OAD bin containing Application and Stack. Intended for OAD.
└── simple_peripheral_cc1350lp_app_stack.hex - BLE Simple Peripheal containing Application and Stack. Intended for OAD download with Device Monitor.

For more information refer the OAD documentation.
