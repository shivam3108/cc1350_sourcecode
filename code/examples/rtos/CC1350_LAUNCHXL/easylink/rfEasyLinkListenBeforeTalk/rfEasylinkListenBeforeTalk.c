/*
 * Copyright (c) 2017-2018, Texas Instruments Incorporated
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

/*
 *  ======== rfEasyLinkListenBeforeTalk.c ========
 */
/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/Assert.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Clock.h>

/* TI-RTOS Header files */
#include <ti/drivers/PIN.h>
#include <ti/drivers/power/PowerCC26XX.h>

/* Board Header files */
#include "Board.h"

/* EasyLink API Header files */
#include "easylink/EasyLink.h"

/* Runtime Library Header Files */
#include <stdlib.h>
#include <string.h>

/* Driverlib Header files */
#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/trng.h)

#define RFEASYLINKLBT_TASK_STACK_SIZE       1024
#define RFEASYLINKLBT_TASK_PRIORITY         2
#define RFEASYLINKLBT_PAYLOAD_LENGTH        30

/*
 * Set to 1 if you want to attempt to retransmit a packet that couldn't be
 * transmitted after the CCA
 */
#define RFEASYLINKLBT_RETRANSMIT_PACKETS    1

#if RFEASYLINKLBT_RETRANSMIT_PACKETS
bool bAttemptRetransmission = false;
#endif // RFEASYLINKLBT_RETRANSMIT_PACKETS

Task_Struct lbtTask;                 /* not static so you can see in ROV */
static Task_Params lbtTaskParams;
static uint8_t lbtTaskStack[RFEASYLINKLBT_TASK_STACK_SIZE];

/* PIN driver handle */
static PIN_Handle pinHandle;
static PIN_State pinState;

/*
 * Application LED pin configuration table:
 *  - All board LEDs are off
 */
PIN_Config pinTable[] = {
    Board_PIN_LED1 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    Board_PIN_LED2 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
#if defined __CC1352R1_LAUNCHXL_BOARD_H__
    Board_DIO30_RFSW | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,
#endif	
    PIN_TERMINATE
};

static uint16_t seqNumber;

static Semaphore_Handle lbtDoneSem;

static uint32_t lastTrngVal;

void HalTRNG_InitTRNG( void )
{
    // configure TRNG
    // Note: Min=4x64, Max=1x256, ClkDiv=1+1 gives the same startup and refill
    //       time, and takes about 11us (~14us with overhead).
    TRNGConfigure( (1 << 8), (1 << 8), 0x01 );

    // enable TRNG
    TRNGEnable();

    // init variable to hold the last value read
    lastTrngVal = 0;
}

void HalTRNG_WaitForReady( void )
{
    // poll status
    while(!(TRNGStatusGet() & TRNG_NUMBER_READY));

    return;
}

uint32_t HalTRNG_GetTRNG( void )
{
    uint32_t trngVal;

    // initialize and enable TRNG if TRNG is not enabled
    if (0 == (HWREG(TRNG_BASE + TRNG_O_CTL) & TRNG_CTL_TRNG_EN))
    {
      HalTRNG_InitTRNG();
    }

    // check that a valid value is ready
    while(!(TRNGStatusGet() & TRNG_NUMBER_READY));

    // check to be sure we're not getting the same value repeatedly
    if ( (trngVal = TRNGNumberGet(TRNG_LOW_WORD)) == lastTrngVal )
    {
      return( 0xDEADBEEF );
    }
    else // value changed!
    {
      // so save last TRNG value
      lastTrngVal = trngVal;

      return( trngVal );
    }
}

void lbtDoneCb(EasyLink_Status status)
{
    if (status == EasyLink_Status_Success)
    {
        /* Toggle LED1 to indicate TX */
        PIN_setOutputValue(pinHandle, Board_PIN_LED1, !PIN_getOutputValue(Board_PIN_LED1));
#if RFEASYLINKLBT_RETRANSMIT_PACKETS
        bAttemptRetransmission = false;
#endif // RFEASYLINKLBT_RETRANSMIT_PACKETS
    }
    else if (status == EasyLink_Status_Busy_Error)
    {
        /* Toggle LED2 to indicate maximum retries reached */
        PIN_setOutputValue(pinHandle, Board_PIN_LED2, !PIN_getOutputValue(Board_PIN_LED2));

#if RFEASYLINKLBT_RETRANSMIT_PACKETS
        bAttemptRetransmission = true;
#endif // RFEASYLINKLBT_RETRANSMIT_PACKETS
    }
    else
    {
        /* Toggle LED1 and LED2 to indicate error */
        PIN_setOutputValue(pinHandle, Board_PIN_LED1, !PIN_getOutputValue(Board_PIN_LED1));
        PIN_setOutputValue(pinHandle, Board_PIN_LED2, !PIN_getOutputValue(Board_PIN_LED2));
    }

    Semaphore_post(lbtDoneSem);
}

static void rfEasyLinkLbtFnx(UArg arg0, UArg arg1)
{
    uint32_t absTime;
    EasyLink_TxPacket lbtPacket = { {0}, 0, 0, {0} };

    /* Create a semaphore */
    Semaphore_Params params;
    Error_Block eb;

    /* Init params */
    Semaphore_Params_init(&params);
    Error_init(&eb);

    /* Create a semaphore instance */
    lbtDoneSem = Semaphore_create(0, &params, &eb);
    if(lbtDoneSem == NULL)
    {
        System_abort("Semaphore creation failed");
    }

    EasyLink_Params easyLink_params;
    EasyLink_Params_init(&easyLink_params);

    easyLink_params.ui32ModType = EasyLink_Phy_Custom;
    easyLink_params.pGrnFxn = (EasyLink_GetRandomNumber)HalTRNG_GetTRNG;

    /* Initialize EasyLink */
    if(EasyLink_init(&easyLink_params) != EasyLink_Status_Success)
    {
        System_abort("EasyLink_init failed");
    }

    /*
     * If you wish to use a frequency other than the default, use
     * the following API:
     * EasyLink_setFrequency(868000000);
     */

    /* Set output power to 12 dBm */
    EasyLink_setRfPower(12);

    while(1)
    {
#if RFEASYLINKLBT_RETRANSMIT_PACKETS
        if(bAttemptRetransmission == false)
        {
#endif // RFEASYLINKLBT_RETRANSMIT_PACKETS
            // zero out the packet only upto the number of payload bytes
            memset(&lbtPacket, 0, sizeof(EasyLink_TxPacket) - (128 - RFEASYLINKLBT_PAYLOAD_LENGTH)*sizeof(uint8_t));

            /* Create packet with incrementing sequence number and random payload */
            lbtPacket.payload[0] = (uint8_t)(seqNumber >> 8);
            lbtPacket.payload[1] = (uint8_t)(seqNumber++);

            uint8_t i;
            for(i = 2; i < RFEASYLINKLBT_PAYLOAD_LENGTH; i++)
            {
                lbtPacket.payload[i] = rand();
            }

            lbtPacket.len = RFEASYLINKLBT_PAYLOAD_LENGTH;
            lbtPacket.dstAddr[0] = 0xaa;

            /* Set Tx absolute time to current time + 100ms */
            if(EasyLink_getAbsTime(&absTime) != EasyLink_Status_Success)
            {
                // Problem getting absolute time
            }
            lbtPacket.absTime = absTime + EasyLink_ms_To_RadioTime(100);
#if RFEASYLINKLBT_RETRANSMIT_PACKETS
        }
#endif // RFEASYLINKLBT_RETRANSMIT_PACKETS

        EasyLink_transmitCcaAsync(&lbtPacket, lbtDoneCb);

        /* Wait forever for TX to complete */
        Semaphore_pend(lbtDoneSem, BIOS_WAIT_FOREVER);
    }
}

void lbtTask_init(PIN_Handle inPinHandle)
{
    pinHandle = inPinHandle;

    Task_Params_init(&lbtTaskParams);
    lbtTaskParams.stackSize = RFEASYLINKLBT_TASK_STACK_SIZE;
    lbtTaskParams.priority  = RFEASYLINKLBT_TASK_PRIORITY;
    lbtTaskParams.stack     = (void *)&lbtTaskStack[0];
    lbtTaskParams.arg0      = (UInt)1000000;

    Task_construct(&lbtTask, rfEasyLinkLbtFnx, &lbtTaskParams, NULL);
}

/*
 *  ======== main ========
 */
int main(void)
{
    /* Call driver init functions */
    Board_initGeneral();

    /* Set power dependency for the TRNG */
    Power_setDependency(PowerCC26XX_PERIPH_TRNG);

    /* Open LED pins */
    pinHandle = PIN_open(&pinState, pinTable);
	Assert_isTrue(pinHandle != NULL, NULL); 

    /* Clear LED pins */
    PIN_setOutputValue(pinHandle, Board_PIN_LED1, 0);
    PIN_setOutputValue(pinHandle, Board_PIN_LED2, 0);

    lbtTask_init(pinHandle);

    /* Start BIOS */
    BIOS_start();

    return(0);
}

