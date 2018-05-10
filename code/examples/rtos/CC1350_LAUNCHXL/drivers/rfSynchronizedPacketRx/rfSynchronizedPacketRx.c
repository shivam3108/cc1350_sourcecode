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

/* Application Header files */
#include "StateMachine.h" 
#include "rfSynchronizedPacket.h"
#include "smartrf_settings/smartrf_settings.h"

/* Board Header files */
#include "Board.h"

/* XDCtools Header files */ 
#include <xdc/std.h>
#include <xdc/runtime/Assert.h>

/* BIOS Header files */ 
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>


/* TI-RTOS Header files */
#include <ti/drivers/pin/PINCC26XX.h>
#include <ti/drivers/rf/RF.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/rf_common_cmd.h)
#include DeviceFamily_constructPath(driverlib/rf_data_entry.h)
#include DeviceFamily_constructPath(driverlib/rf_mailbox.h)
#include DeviceFamily_constructPath(driverlib/rf_prop_cmd.h)
#include DeviceFamily_constructPath(driverlib/rf_prop_mailbox.h)


/* Define events that can be posted to the application state machine */
typedef enum {
    Event_SyncButtonPushed = StateMachine_Event00,
    Event_PacketReceived = StateMachine_Event01,
    Event_SyncMissed = StateMachine_Event02
} Event;

/* Declare state handler functions for the application. */
StateMachine_DECLARE_STATE(SetupState);
StateMachine_DECLARE_STATE(WaitingForSyncState);
StateMachine_DECLARE_STATE(SyncedRxState);


/*
 * Application LED pin configuration table:
 *   - All LEDs board LEDs are off.
 *   - Button pins are high by default and pulled to low when pressed.
 */
PIN_Config pinTable[] =
{
    Board_PIN_LED1 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    Board_PIN_LED2 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    Board_PIN_BUTTON0 | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE,
#if defined __CC1352R1_LAUNCHXL_BOARD_H__
    Board_DIO30_RFSW | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,
#endif
	PIN_TERMINATE
};


/***** Defines *****/
#define MAIN_TASK_STACK_SIZE 1024
#define MAIN_TASK_PRIORITY   2

/* Packet RX Configuration */
#define NUM_APPENDED_BYTES     5  /* The Data Entries data field will contain:
                                   * - 1 address byte in the header (RF_cmdPropRx.rxConf.bIncludeHdr = 0x1)
                                   * - up to sizeof BeaconPacket bytes payload
                                   * - 4 bytes for the RAT time stamp (RF_cmdPropRx.rxConf.bAppendTimestamp = 0x1) */

#define SYMBOL_RATE            50000                    /* 50 kbits per second */
#define US_PER_SYMBOL          (1000000 / SYMBOL_RATE)
#define PREAMBLE_BITS          32
#define SYNCWORD_BITS          32

#define RX_START_MARGIN        RF_convertUsToRatTicks(500)   /* An arbitrarily chosen value to compensate for
                                                              * the potential drift of the RAT and the RTC. */

#define RX_TIMEOUT_TICKS       RF_convertUsToRatTicks((PREAMBLE_BITS + SYNCWORD_BITS) * US_PER_SYMBOL)
                                                             /* Tight, but ideal duration for receiving all bits of
                                                              * the preamble and the sync word. */

#define RX_TIMEOUT_MARGIN      RF_convertUsToRatTicks(1000)  /* Arbitrarily chosen margin added to the RX timeout
                                                              * to compensate calculation errors. */

#define RX_START_TO_SETTLE_TICKS   256 /* Time between RX start trigger and the radio
                                        * being ready to receive the first preamble bit.
                                        * This is a fixed value for CMD_PROP_RX. */
#define TX_START_TO_PREAMBLE_TICKS 384 /* Time between TX start trigger and first bit on air.
                                        * This is a fixed value for CMD_PROP_TX. */


/***** Prototypes *****/
void buttonCallbackFunction(PIN_Handle handle, PIN_Id pinId);
void WaitingForSyncState_rxCallback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e);
void SyncedRxState_rxCallback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e);

/***** Variable declarations *****/
Task_Struct mainTask;    /* not static so you can see in ROV */
static uint8_t mainTaskStack[MAIN_TASK_STACK_SIZE];

static StateMachine_Struct stateMachine;

static PIN_Handle pinHandle;
static PIN_State pinState;

static RF_Object rfObject;
static RF_Handle rfHandle;

/* Queue object that the RF Core will fill with data */
static dataQueue_t rxQueue;

/* A single queue item that points to a data buffer  */
static rfc_dataEntryPointer_t rxItem;

/* Word-aligned buffer for the packet payload + meta data. */
static uint8_t rxBuffer[((sizeof(BeaconPacket) + NUM_APPENDED_BYTES) + 8)];

BeaconPacket beacon;

/***** Function definitions *****/
int main(void)
{
    /* Call driver init functions. */
    Board_initGeneral();

    /* Open LED pins */
    pinHandle = PIN_open(&pinState, pinTable);
    Assert_isTrue(pinHandle != NULL, NULL);

    /* Setup callback for button pins */
    PIN_Status status = PIN_registerIntCb(pinHandle, &buttonCallbackFunction);
    Assert_isTrue(status == PIN_SUCCESS, NULL);

    // Initialise the application state machine.
    StateMachine_construct(&stateMachine);

    /* Initialise the main task. It will execute
     * the state machine StateMachine_exec() function.
     */
    Task_Params params;
    Task_Params_init(&params);
    params.stackSize = MAIN_TASK_STACK_SIZE;
    params.priority = MAIN_TASK_PRIORITY;
    params.stack = &mainTaskStack;
    params.arg0 = (UArg)&stateMachine;
    params.arg1 = (UArg)SetupState;
    Task_construct(&mainTask, (Task_FuncPtr)&StateMachine_exec, &params, NULL);

    /* Start BIOS */
    BIOS_start();

    return (0);
}

/* Pin interrupt Callback function board buttons configured in the pinTable. */
void buttonCallbackFunction(PIN_Handle handle, PIN_Id pinId) {

    /* Debounce the button with a short delay */
    CPUdelay(CPU_convertMsToDelayCycles(10));
    if (PIN_getInputValue(pinId) == 1)
    {
        return;
    }

    switch (pinId)
    {
    case Board_PIN_BUTTON0:
        StateMachine_postEvents(&stateMachine, Event_SyncButtonPushed);
        break;
    }
}

void SetupState_function()
{
    /* Construct a circular RX queue with a single pointer-entry item. */
    rxItem.config.type = DATA_ENTRY_TYPE_PTR;
    rxItem.config.lenSz = 0;
    rxItem.length = sizeof(rxBuffer);
    rxItem.pNextEntry = (uint8_t*)&rxItem;
    rxItem.pData = (uint8_t*)&rxBuffer[0];
    rxItem.status = DATA_ENTRY_PENDING;
    rxQueue.pCurrEntry = (uint8_t*)&rxItem;
    rxQueue.pLastEntry = NULL;

    /* Modify CMD_PROP_RX command for application needs */
    RF_cmdPropRx.pQueue = &rxQueue;                /* Set the Data Entity queue for received data */
    RF_cmdPropRx.rxConf.bAutoFlushIgnored = true;  /* Discard ignored packets from Rx queue */
    RF_cmdPropRx.rxConf.bAutoFlushCrcErr = true;   /* Discard packets with CRC error from Rx queue */
    RF_cmdPropRx.rxConf.bIncludeHdr = true;        /* Put length field in front of queue entries. */
    RF_cmdPropRx.rxConf.bAppendTimestamp = true;   /* Append RX time stamp to the packet payload */
    RF_cmdPropRx.maxPktLen = sizeof(BeaconPacket); /* Implement packet length filtering to avoid PROP_ERROR_RXBUF */
    RF_cmdPropRx.pktConf.bRepeatOk = false;        /* Stop after receiving a single valid packet */
    RF_cmdPropRx.pktConf.bRepeatNok = true;

    /* Request access to the radio. This does not power-up the RF core, but only initialise
     * the driver and cache the setup command. */
    RF_Params rfParams;
    RF_Params_init(&rfParams);
    rfHandle = RF_open(&rfObject, &RF_prop, (RF_RadioSetup*)&RF_cmdPropRadioDivSetup, &rfParams);
    
    Assert_isTrue(rfHandle != NULL, NULL);

    /* Set the frequency. Now the RF driver powers the RF core up and runs the setup command from above.
     * The FS command is executed and also cached for later use when the RF driver does an automatic
     * power up. */
    RF_EventMask result = RF_runCmd(rfHandle, (RF_Op*)&RF_cmdFs, RF_PriorityNormal, NULL, 0);
    Assert_isTrue(((result == RF_EventLastCmdDone) && ((volatile RF_Op*)&RF_cmdFs)->status == DONE_OK),NULL);

    /* Route the LNA signal to an LED to indicate that the RF core is
     * active and receiving data.
     * Available signals are listed in the proprietary RF user's guide.
     */
    PINCC26XX_setMux(pinHandle, Board_PIN_LED2, PINCC26XX_MUX_RFC_GPO0);

    StateMachine_setNextState(&stateMachine, WaitingForSyncState);

}

void WaitingForSyncState_function()
{
    rxItem.status = DATA_ENTRY_PENDING;

    /* Start RX command to receive a single packet. */
    RF_cmdPropRx.startTrigger.triggerType = TRIG_NOW;
    RF_cmdPropRx.endTrigger.triggerType = TRIG_NEVER;
    RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropRx, RF_PriorityNormal, &WaitingForSyncState_rxCallback, RF_EventRxEntryDone);

    for (;;)
    {
        StateMachine_EventMask events = StateMachine_pendEvents(&stateMachine, Event_PacketReceived, BIOS_WAIT_FOREVER);

        if (events & Event_PacketReceived)
        {
            /* RX command has already stopped. Now examine the received data. */
            uint8_t length;
            uint32_t rxTime;

            memcpy(&length, rxBuffer, 1);
            if (length != sizeof(BeaconPacket))
            {
                // This packet is not for us. Wait for next one
                StateMachine_setNextState(&stateMachine, WaitingForSyncState);
                break;
            }

            memcpy(&beacon, (uint8_t*)rxBuffer + 1, sizeof(BeaconPacket));
            memcpy(&rxTime, (uint8_t*)rxBuffer + 1 + sizeof(BeaconPacket), 4);

            PIN_setOutputValue(pinHandle, Board_PIN_LED1, beacon.ledState);

            /* rxTime contains a calculated time stamp when the first preamble byte
             * was sent on air. As a time base for the next wake ups, we calculate
             * the time when this RX command would have been started for a synchronised
             * wake up. */

            RF_cmdPropRx.startTime = rxTime - RX_START_TO_SETTLE_TICKS - RX_START_MARGIN;

            StateMachine_setNextState(&stateMachine, SyncedRxState);
            break;
        }
    }
}

void WaitingForSyncState_rxCallback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
    if (e & RF_EventLastCmdDone)
    {
        if ((RF_cmdPropRx.status == PROP_DONE_OK) || (RF_cmdPropRx.status == PROP_DONE_ENDED))
        {
            /* Sync word has been found before end trigger and packet has been received */
            StateMachine_postEvents(&stateMachine, Event_PacketReceived);
        }
        else
        {
            // Everything else is an error.
            Assert_isTrue(false, NULL);
        }
    }
}

void SyncedRxState_function()
{
    rxItem.status = DATA_ENTRY_PENDING;

    /* Start RX command to receive a single packet. Use an absolute start trigger
     * and a predicted start time. The end time is calculated to be as tight as possible.
     * The compensation margin for RAT drift in both directions needs to be taken into account.  */
    RF_cmdPropRx.startTrigger.triggerType = TRIG_ABSTIME;
    RF_cmdPropRx.startTime += beacon.beaconInterval;
    RF_cmdPropRx.endTrigger.triggerType = TRIG_REL_START;
    RF_cmdPropRx.endTime = RX_START_TO_SETTLE_TICKS + RX_TIMEOUT_TICKS + RX_TIMEOUT_MARGIN + RX_START_MARGIN;

    /* Puts the RX command into the driver queue. Since the start trigger is absolute and has
     * a time somewhere in the future, the RF driver will power down the RF core and wait short
     * before the RX command is due. Then it will run the power-up sequence and dispatch the RX
     * command right on time.
     */
    RF_CmdHandle cmd = RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropRx, RF_PriorityNormal, &SyncedRxState_rxCallback, RF_EventRxEntryDone);

    for (;;)
    {
        StateMachine_EventMask events = StateMachine_pendEvents(&stateMachine, Event_PacketReceived | Event_SyncButtonPushed | Event_SyncMissed, BIOS_WAIT_FOREVER);

        if (events & Event_PacketReceived)
        {
            /* RX command has already stopped. Now examine the received data. */
            uint8_t length;
            uint32_t rxTime;

            memcpy(&length, rxBuffer, 1);
            if (length != sizeof(BeaconPacket))
            {
                // This packet is not for us. Wait for next one
                StateMachine_setNextState(&stateMachine, WaitingForSyncState);
                break;
            }
            memcpy(&beacon, (uint8_t*)rxBuffer + 1, sizeof(BeaconPacket));
            memcpy(&rxTime, (uint8_t*)rxBuffer + 1 + sizeof(BeaconPacket), 4);

            /* The synchronisation offset might be used to calculate the clock drift between tranStateMachine_itter and receiver. */
            int32_t syncOffsetTime = rxTime - RX_START_TO_SETTLE_TICKS - RF_cmdPropRx.startTime;
            (void)syncOffsetTime; // need to reference the variable to prevent from a compiler warning

            /* Do an immediate re-synchronisation based on the new RX time. */
            RF_cmdPropRx.startTime = rxTime - RX_START_TO_SETTLE_TICKS - RX_START_MARGIN;

            /* Display the current LED state of the TX board. */
            PIN_setOutputValue(pinHandle, Board_PIN_LED1, beacon.ledState);

            /* Wait for the next packet */
            StateMachine_setNextState(&stateMachine, SyncedRxState);
            break;
        }

        if (events & Event_SyncMissed)
        {
            /* Sync is missed. That means either we are out of sync or the
             * tranStateMachine_itter is in spontaneous mode. Try to receive the next packet. */
            StateMachine_setNextState(&stateMachine, SyncedRxState);
            break;
        }

        if (events & Event_SyncButtonPushed)
        {
            /* Force re-synchronisation */
            RF_cancelCmd(rfHandle, cmd, 0);
            RF_pendCmd(rfHandle, cmd, RF_EventCmdCancelled);
            StateMachine_setNextState(&stateMachine, WaitingForSyncState);
            break;
        }
    }
}

void SyncedRxState_rxCallback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
    if (e & RF_EventLastCmdDone)
    {
        if ((RF_cmdPropRx.status == PROP_DONE_OK) || (RF_cmdPropRx.status == PROP_DONE_ENDED))
        {
            /* Sync word has been found before end trigger and packet has been received */
            StateMachine_postEvents(&stateMachine, Event_PacketReceived);
        }
        else if (RF_cmdPropRx.status == PROP_DONE_RXTIMEOUT)
        {
            StateMachine_postEvents(&stateMachine, Event_SyncMissed);
        }
    }
}
