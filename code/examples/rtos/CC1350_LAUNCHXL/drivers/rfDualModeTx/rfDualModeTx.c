/*
 * Copyright (c) 2017, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/***** Includes *****/
/* Standard C Libraries */
#include <stdlib.h>

/* TI Drivers */
#include <ti/drivers/rf/RF.h>
#include <ti/drivers/PIN.h>

/* Driverlib Header files */
#include DeviceFamily_constructPath(driverlib/rf_prop_mailbox.h)
#include DeviceFamily_constructPath(driverlib/rf_ble_mailbox.h)

/* Board Header files */
#include "Board.h"

/* Application Header files */
#include "smartrf_settings/smartrf_settings.h"
#include "smartrf_settings/smartrf_settings_ble.h"

/***** Defines *****/
/*
 * BLE PACKET
 * ________________________________________________________________________________________________________________________
 *|                                 |               |                                 |          |           |             |
 *| PDU HEADER FIELD             2B | ADV ADDR      | PACKET DATA                 27B | CRC   3B | RSSI   1B | STATUS   2B |
 *| ADV TYPE   1B | PACKET LEN   1B |  - BLE4    6B | SERIAL NUMBER   2B | DATA   25B |          |           |             |
 *|                                 |  - BLE5   10B |                                 |          |           |             |
 *|_________________________________|_______________|_________________________________|__________|___________|_____________|
 *
 * PROPRIETARY PACKET
 * _____________________________________________________________
 *|                |                                 |          |
 *| LEN FIELD   1B | PACKET DATA                 27B | CRC   2B |
 *|                | SERIAL NUMBER   2B | DATA   25B |          |
 *|________________|_________________________________|__________|
 *
 */
#if defined(__CC1352R1_LAUNCHXL_BOARD_H__)
#define RFSW_PIN                          Board_DIO30_RFSW
#else
#define RFSW_PIN                          Board_DIO1_RFSW
#endif

#define PROP_SWITCH_SETTING               1
#define BLE_SWITCH_SETTING                0

#if defined(__CC1352R1_LAUNCHXL_BOARD_H__)
#define EXTENDED_HEADER_LENGTH     		  9
#endif

/* Packet TX Configuration */
#define PACKET_DATA_LENGTH               	  27 /* Packet for BLE5 cannot exceed 37 with header (10) */
#define PACKET_INTERVAL     (uint32_t)(4000000*0.5f) /* Set packet interval to 500ms */

/***** Variable declarations *****/
static RF_Object rfBleObject;
static RF_Handle rfBleHandle;

static RF_Object rfPropObject;
static RF_Handle rfPropHandle;

/* Pin driver handle */
static PIN_Handle pinHandle;
static PIN_State pinState;

static uint8_t packet[PACKET_DATA_LENGTH];
static uint16_t seqNumber;

/* Packet TX struct for BLE5 */
#if defined(__CC2640R2_LAUNCHXL_BOARD_H__) || defined(__CC1352R1_LAUNCHXL_BOARD_H__) || defined(__CC26X2R1_LAUNCHXL_BOARD_H__)
rfc_ble5ExtAdvEntry_t ble5ExtAdvPacket;
#endif

/*
 * Application LED pin configuration table:
 *   - All LEDs board LEDs are off.
 *   - Antenna is set to sub 1 mode.
 */
PIN_Config pinTable[] =
{
 RFSW_PIN | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,
#if defined(__CC1350_LAUNCHXL_BOARD_H__)
 Board_DIO30_SWPWR | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,
#endif
 Board_PIN_LED1 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
 Board_PIN_LED2 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
 PIN_TERMINATE
};

/***** Function definitions *****/
static void txDoneCallback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
    if (e & RF_EventLastCmdDone)
    {
        /* Successful TX */
        if (h == rfPropHandle)
        {
            /* Toggle LED2, clear LED1 to indicate Prop TX */
            PIN_setOutputValue(pinHandle, Board_PIN_LED2, 1);
            PIN_setOutputValue(pinHandle, Board_PIN_LED1, 0);

        }
        else
        {
            /* Toggle LED1, clear LED2 to indicate Ble TX */
            PIN_setOutputValue(pinHandle, Board_PIN_LED2, 0);
            PIN_setOutputValue(pinHandle, Board_PIN_LED1, 1);
        }
    }
    else
    {
        /* Error Condition: set both LEDs */
        PIN_setOutputValue(pinHandle, Board_PIN_LED1, 1);
        PIN_setOutputValue(pinHandle, Board_PIN_LED2, 1);
    }
}

static void txSwitchClient(RF_Handle h, RF_ClientEvent event, void* arg)
{
    /* Switch antenna */
    if (h == rfPropHandle)
    {
        PIN_setOutputValue(pinHandle, RFSW_PIN, PROP_SWITCH_SETTING);
    }
    else
    {
        PIN_setOutputValue(pinHandle, RFSW_PIN, BLE_SWITCH_SETTING);
    }
}

void *mainThread(void *arg0)
{
    uint32_t curtime;
    uint32_t cmdStatus;
    RF_EventMask terminationReason;

    pinHandle = PIN_open(&pinState, pinTable);
    if (pinHandle == NULL)
    {
        while(1);
    }

    /* Initialize multimode scheduling params
     * - Params shared between rf drivers since commands are synchronous
     * - Ignore end time
     * - Priority should not effect transmission */
    RF_ScheduleCmdParams schParams;
    schParams.endTime = 0;
    schParams.priority = RF_PriorityNormal;

#if defined(__CC1352R1_LAUNCHXL_BOARD_H__) || defined(__CC26X2R1_LAUNCHXL_BOARD_H__)
    schParams.allowDelay = RF_AllowDelayAny;
#endif
    /* Initialize Prop RF Driver */
    RF_Params rfPropParams;
    RF_Params_init(&rfPropParams);

    /* Set mode for multiple clients
     * - Configure client event mask
     * - Set callback for client switch */
    RF_prop.rfMode = RF_MODE_MULTIPLE;
    rfPropParams.nClientEventMask = RF_ClientEventSwitchClientEntered;
    rfPropParams.pClientEventCb = txSwitchClient;

    RF_cmdPropTx.pktLen = PACKET_DATA_LENGTH;
    RF_cmdPropTx.pPkt = packet;
    RF_cmdPropTx.startTrigger.triggerType = TRIG_ABSTIME;
    RF_cmdPropTx.startTrigger.pastTrig = 1;
    RF_cmdPropTx.startTime = 0;

    /* Request access to the prop radio and
     * - Radio is not powered on by RF_open
     * - RF_cmdFs will power on the radio to cache the frequency settings */
    rfPropHandle = RF_open(&rfPropObject, &RF_prop,
                           (RF_RadioSetup*) &RF_cmdPropRadioDivSetup,
                           &rfPropParams);
    RF_postCmd(rfPropHandle, (RF_Op*) &RF_cmdFs, RF_PriorityNormal, NULL, 0);

    /* Initialize BLE RF Driver */
    RF_Params rfBleParams;
    RF_Params_init(&rfBleParams);

    /* Set mode for multiple clients
     * - Configure client event mask
     * - Set callback for client switch */
    RF_pModeBle->rfMode = RF_MODE_MULTIPLE;
    rfBleParams.nClientEventMask = RF_ClientEventSwitchClientEntered;
    rfBleParams.pClientEventCb = txSwitchClient;

    /* Request access to the ble radio */
#if defined(__CC2640R2_LAUNCHXL_BOARD_H__) || defined(__CC1352R1_LAUNCHXL_BOARD_H__) || defined(__CC26X2R1_LAUNCHXL_BOARD_H__)
    RF_ble_pCmdBle5AdvAux->pParams->pAdvPkt = (uint8_t *)&ble5ExtAdvPacket;
    ble5ExtAdvPacket.extHdrInfo.length = EXTENDED_HEADER_LENGTH;
    ble5ExtAdvPacket.advDataLen = PACKET_DATA_LENGTH;
    ble5ExtAdvPacket.pAdvData = packet;
    RF_ble_pCmdBle5AdvAux->startTrigger.triggerType = TRIG_ABSTIME;
    RF_ble_pCmdBle5AdvAux->startTrigger.pastTrig = 1;
    RF_ble_pCmdBle5AdvAux->startTime = 0;
#else
    RF_ble_pCmdBleAdvNc->pParams->advLen = PACKET_DATA_LENGTH;
    RF_ble_pCmdBleAdvNc->pParams->pAdvData = packet;
    RF_ble_pCmdBleAdvNc->startTrigger.triggerType = TRIG_ABSTIME;
    RF_ble_pCmdBleAdvNc->startTrigger.pastTrig = 1;
    RF_ble_pCmdBleAdvNc->startTime = 0;
#endif

    /* Request access to the bleradio and
     * - RF_ble_pCmdFs does not need to run unless no channel is specified (0xFF)
     * - Channel 17 (0x8C) is used by default */
    rfBleHandle = RF_open(&rfBleObject, RF_pModeBle,
                          (RF_RadioSetup*) RF_ble_pCmdRadioSetup, &rfBleParams);
    //RF_runCmd(rfBleHandle, (RF_Op*)RF_ble_pCmdFs, RF_PriorityNormal, NULL, 0);


    /* Get current time */
    curtime = RF_getCurrentTime();

    while (1)
    {
        /* Create packet with incrementing sequence number and random payload */
        packet[0] = (uint8_t) (seqNumber >> 8);
        packet[1] = (uint8_t) (seqNumber++);
        uint8_t i;
        for (i = 2; i < PACKET_DATA_LENGTH; i++)
        {
            packet[i] = rand();
        }

        /* Set absolute TX time to utilize automatic power management */
        curtime += PACKET_INTERVAL;

        /* Transmit prop packet */
        RF_cmdPropTx.startTime = curtime;

        terminationReason = RF_runScheduleCmd(rfPropHandle, (RF_Op*) &RF_cmdPropTx,
                                      &schParams, txDoneCallback, 0);
        switch (terminationReason)
        {
            case RF_EventCmdDone:
                // A radio operation command in a chain finished
                break;
            case RF_EventLastCmdDone:
                // A stand-alone radio operation command or the last radio
                // operation command in a chain finished.
                break;
            case RF_EventCmdCancelled:
                // Command cancelled before it was started; it can be caused
                // by RF_cancelCmd() or RF_flushCmd().
                break;
            case RF_EventCmdAborted:
                // Abrupt command termination caused by RF_cancelCmd() or
                // RF_flushCmd().
                break;
            case RF_EventCmdStopped:
                // Graceful command termination caused by RF_cancelCmd() or
                // RF_flushCmd().
                break;
            default:
                // Uncaught error event
                while (1);
        }

        cmdStatus = ((volatile RF_Op*) &RF_cmdPropTx)->status;

        switch (cmdStatus)
        {
            case PROP_DONE_OK:
                // Packet transmitted successfully
                break;
            case PROP_DONE_STOPPED:
                // received CMD_STOP while transmitting packet and finished
                // transmitting packet
                break;
            case PROP_DONE_ABORT:
                // Received CMD_ABORT while transmitting packet
                break;
            case PROP_ERROR_PAR:
                // Observed illegal parameter
                break;
            case PROP_ERROR_NO_SETUP:
                // Command sent without setting up the radio in a supported
                // mode using CMD_BLE_RADIO_SETUP or CMD_RADIO_SETUP
                break;
            case PROP_ERROR_NO_FS:
                // Command sent without the synthesizer being programmed
                break;
            case PROP_ERROR_TXUNF:
                // TX underflow observed during operation
                break;
            default:
                // Uncaught error event - these could come from the
                // pool of states defined in rf_mailbox.h
                while (1);
        }

        /* Transmit BLE packet */
        /* Set absolute TX time to utilize automatic power management */
        curtime += PACKET_INTERVAL;

        /* Switch from PROP -> BLE client */
#if (defined __CC2640R2_LAUNCHXL_BOARD_H__) || (defined __CC1352R1_LAUNCHXL_BOARD_H__) || (defined __CC26X2R1_LAUNCHXL_BOARD_H__)
        RF_ble_pCmdBle5AdvAux->startTime = curtime;
        terminationReason = RF_runScheduleCmd(rfBleHandle, (RF_Op*)RF_ble_pCmdBle5AdvAux,
                                              &schParams, txDoneCallback, 0);
#else
        RF_ble_pCmdBleAdvNc->startTime = curtime;
        terminationReason = RF_runScheduleCmd(rfBleHandle, (RF_Op*) RF_ble_pCmdBleAdvNc,
                                              &schParams, txDoneCallback, 0);
#endif

        switch (terminationReason)
        {
            case RF_EventCmdDone:
                // A radio operation command in a chain finished
                break;
            case RF_EventLastCmdDone:
                // A stand-alone radio operation command or the last radio
                // operation command in a chain finished.
                break;
            case RF_EventCmdCancelled:
                // Command cancelled before it was started; it can be caused
                // by RF_cancelCmd() or RF_flushCmd().
                break;
            case RF_EventCmdAborted:
                // Abrupt command termination caused by RF_cancelCmd() or
                // RF_flushCmd().
                break;
            case RF_EventCmdStopped:
                // Graceful command termination caused by RF_cancelCmd() or
                // RF_flushCmd().
                break;
            default:
                // Uncaught error event
                while (1);
        }

#if (defined __CC2640R2_LAUNCHXL_BOARD_H__) || (defined __CC1352R1_LAUNCHXL_BOARD_H__) || (defined __CC26X2R1_LAUNCHXL_BOARD_H__)
        cmdStatus = RF_ble_pCmdBle5AdvAux->status;
#else
        cmdStatus = RF_ble_pCmdBleAdvNc->status;
#endif

        switch (cmdStatus)
        {
            case BLE_DONE_OK:
                // Packet transmitted successfully
                break;
            case BLE_DONE_STOPPED:
                // received CMD_STOP while transmitting packet and finished
                // transmitting packetoi
                break;
            case BLE_DONE_ABORT:
                // Received CMD_ABORT while transmitting packet
                break;
            case BLE_ERROR_PAR:
                // Observed illegal parameter
                break;
            case BLE_ERROR_NO_SETUP:
                // Command sent without setting up the radio in a supported
                // mode using CMD_BLE_RADIO_SETUP or CMD_RADIO_SETUP
                break;
            case BLE_ERROR_NO_FS:
                // Command sent without the synthesizer being programmed
                break;
            case BLE_ERROR_TXUNF:
                // TX underflow observed during operation
                break;
            default:
                // Uncaught error event - these could come from the
                // pool of states defined in rf_mailbox.h
                while (1);
        }
    }
}
