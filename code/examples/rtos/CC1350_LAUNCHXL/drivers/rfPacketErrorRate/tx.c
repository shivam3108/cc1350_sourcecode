/*
 * Copyright (c) 2016-2017, Texas Instruments Incorporated
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

/* TI-RTOS Header files */
#include <ti/drivers/rf/RF.h>
#include <ti/drivers/PIN.h>

/* Board Header files */
#include "Board.h"

/* Application specific Header files */
#include "menu.h"
#include "RFQueue.h"
#include "smartrf_settings/smartrf_settings.h"
#include "smartrf_settings/smartrf_settings_predefined.h"

#if (defined __CC2650DK_7ID_BOARD_H__)        || (defined __CC2650_LAUNCHXL_BOARD_H__)   || \
    (defined __CC2640R2_LAUNCHXL_BOARD_H__)   || (defined __CC1350_LAUNCHXL_BOARD_H__)   || \
    (defined __CC1350_LAUNCHXL_433_BOARD_H__) || (defined __CC1350STK_BOARD_H__)         || \
    (defined __CC1352R1_LAUNCHXL_BOARD_H__)   || (defined __CC1352P1_LAUNCHXL_BOARD_H__) || \
    (defined __CC26X2R1_LAUNCHXL_BOARD_H__)
#include "smartrf_settings/smartrf_settings_ble.h"
#endif

/***** Defines *****/
#define MAX_PAYLOAD_LENGTH      254 // Maximum length of the packet to send (Even due to HS requirement)
#define MAX_BLE_PAYLOAD_LENGTH  30  // Maximum length of the BLE4/5 packet to send
#define DATA_ENTRY_HEADER_SIZE  8   // Constant header size of a Generic Data Entry
#define MAX_LENGTH              254 // Set the length of the data entry
#define NUM_DATA_ENTRIES        1
#define NUM_APPENDED_BYTES      0

#define EXTENDED_HEADER_LENGTH  9
#define BLE_BASE_FREQUENCY      2300 // When programming the channel in the BLE TX command it is the
                                     // offset from 2300 MHz

#define ABORT_GRACEFUL          1   // Option for the RF cancel command
#define ABORT_ABRUPT            0   // Option for the RF cancel command

/* Inter-packet intervals for each phy mode in ms*/
#define PKT_INTERVAL_MS_2GFSK   60
#define PKT_INTERVAL_MS_CUSTOM  60
#define PKT_INTERVAL_MS_SLR     80
#define PKT_INTERVAL_MS_LRM     500
#define PKT_INTERVAL_MS_OOK     100
#define PKT_INTERVAL_MS_HSM     50
#define PKT_INTERVAL_MS_BLE     100

#define RF_TX20_ENABLED         0xFFFF // Tx power setting when high PA is in use
#define CENTER_FREQ_EU          0x0364 // Center Frequency 868 MHz
#define CENTER_FREQ_US          0x0393 // Center Frequency 915 MHz

/***** Prototypes *****/
static void tx_callback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e);

/***** Variable declarations *****/
static RF_Object rfObject;
static RF_Handle rfHandle;

static uint8_t packet[MAX_PAYLOAD_LENGTH];
static volatile uint16_t seqNumber = 0;
static volatile uint32_t packetCount = 0;
static ApplicationConfig localConfig;
static volatile uint32_t time = 0;
static volatile bool bPacketTxDone = false;
static volatile RF_CmdHandle cmdHandle;

static uint8_t triggerType = TRIG_NOW;

static tx_metrics txMetrics = {
    .transmitPowerDbm = -128,
    .dataRateBps      = 0,
    .packetIntervalMs = 0
};

/*
This interval is dependent on data rate and packet length, and might need to be changed
if any of these parameter changes
*/
uint32_t packetInterval;

#if defined(__TI_COMPILER_VERSION__)
    #pragma DATA_ALIGN (txDataEntryBuffer, 4);
        static uint8_t txDataEntryBuffer[RF_QUEUE_DATA_ENTRY_BUFFER_SIZE(NUM_DATA_ENTRIES,
                                                                        MAX_LENGTH,
                                                                        NUM_APPENDED_BYTES)];
#elif defined(__IAR_SYSTEMS_ICC__)
    #pragma data_alignment = 4
        static uint8_t txDataEntryBuffer[RF_QUEUE_DATA_ENTRY_BUFFER_SIZE(NUM_DATA_ENTRIES,
                                                                        MAX_LENGTH,
                                                                        NUM_APPENDED_BYTES)];
#elif defined(__GNUC__)
        static uint8_t txDataEntryBuffer [RF_QUEUE_DATA_ENTRY_BUFFER_SIZE(NUM_DATA_ENTRIES,
                                                                        MAX_LENGTH,
                                                                        NUM_APPENDED_BYTES)] __attribute__ ((aligned (4)));
#else
    #error This compiler is not supported.
#endif

/* TX queue or RF Core to read data from */
static dataQueue_t dataQueue;
static rfc_dataEntryGeneral_t* currentDataEntry;
static uint8_t *pPacket;
#if (defined __CC2640R2_LAUNCHXL_BOARD_H__) || (defined __CC1352R1_LAUNCHXL_BOARD_H__) || \
    (defined __CC1352P1_LAUNCHXL_BOARD_H__) || (defined __CC26X2R1_LAUNCHXL_BOARD_H__)
rfc_ble5ExtAdvEntry_t ble5ExtAdvPacket;
#endif

/* Runs the transmitting part of the test application and returns a result. */
TestResult tx_runTxTest(const ApplicationConfig* config)
{
    uint32_t lastpacketCount = 0;
    uint16_t cmdTxPower      = 0;
#if (defined __CC1310_LAUNCHXL_BOARD_H__)   || (defined __CC1350_LAUNCHXL_BOARD_H__)     || \
    (defined __CC1350STK_BOARD_H__)         || (defined __CC1350_LAUNCHXL_433_BOARD_H__) || \
    (defined __CC1312R1_LAUNCHXL_BOARD_H__) || (defined __CC1352R1_LAUNCHXL_BOARD_H__)   || \
    (defined __CC1352P1_LAUNCHXL_BOARD_H__)
    RF_TxPowerTable_Entry *rfPowerTable = NULL;
    uint8_t rfPowerTableSize = 0;
#endif

    if(config == NULL)
    {
        while(1);
    }
    memcpy((void *)&localConfig, config, sizeof(ApplicationConfig));

    RF_Params rfParams;
    RF_Params_init(&rfParams);
    if(localConfig.intervalMode == IntervalMode_Yes)
    {
        triggerType = TRIG_ABSTIME;
    }
    else
    {
        triggerType = TRIG_NOW;
    }

    RF_cmdPropTx.pktLen = config->payloadLength;
    RF_cmdPropTx.pPkt = packet;
    RF_cmdPropTx.startTrigger.triggerType = triggerType;
    RF_cmdPropTx.startTrigger.pastTrig = 1;
    RF_cmdPropTx.startTime = 0;

    if( RFQueue_defineQueue(&dataQueue,
                            txDataEntryBuffer,
                            sizeof(txDataEntryBuffer),
                            NUM_DATA_ENTRIES,
                            MAX_LENGTH + NUM_APPENDED_BYTES))
    {
        /* Failed to allocate space for all data entries */
        while(true);
    }

#if !(defined __CC2650DK_7ID_BOARD_H__)      && !(defined __CC2650_LAUNCHXL_BOARD_H__)     && \
    !(defined __CC2640R2_LAUNCHXL_BOARD_H__) && !(defined __CC1350_LAUNCHXL_433_BOARD_H__) && \
    !(defined __CC1352R1_LAUNCHXL_BOARD_H__) && !(defined __CC1352P1_LAUNCHXL_BOARD_H__)   && \
    !(defined __CC26X2R1_LAUNCHXL_BOARD_H__) && !(defined __CC1312R1_LAUNCHXL_BOARD_H__)
    RF_pCmdTxHS->pQueue = &dataQueue;
    RF_pCmdTxHS->startTrigger.triggerType  = triggerType;
    RF_pCmdTxHS->startTrigger.pastTrig = 1;
    RF_pCmdTxHS->startTime = 0;
#endif

#if (defined __CC2650DK_7ID_BOARD_H__)        || (defined __CC2650_LAUNCHXL_BOARD_H__)   || \
    (defined __CC2640R2_LAUNCHXL_BOARD_H__)   || (defined __CC1350_LAUNCHXL_BOARD_H__)   || \
    (defined __CC1350_LAUNCHXL_433_BOARD_H__) || (defined __CC1350STK_BOARD_H__)         || \
    (defined __CC26X2R1_LAUNCHXL_BOARD_H__)   || (defined __CC1352R1_LAUNCHXL_BOARD_H__) || \
    (defined __CC1352P1_LAUNCHXL_BOARD_H__)
#if defined(__CC2640R2_LAUNCHXL_BOARD_H__)  || defined(__CC1352R1_LAUNCHXL_BOARD_H__) || \
    (defined __CC1352P1_LAUNCHXL_BOARD_H__) || defined(__CC26X2R1_LAUNCHXL_BOARD_H__)
    RF_ble_pCmdBle5AdvAux->pParams->pAdvPkt = (uint8_t *)&ble5ExtAdvPacket;
    ble5ExtAdvPacket.extHdrInfo.length = EXTENDED_HEADER_LENGTH;
    ble5ExtAdvPacket.advDataLen = MAX_BLE_PAYLOAD_LENGTH - EXTENDED_HEADER_LENGTH - 1;
    ble5ExtAdvPacket.pAdvData = packet;
    RF_ble_pCmdBle5AdvAux->startTrigger.triggerType  = triggerType;
    RF_ble_pCmdBle5AdvAux->startTrigger.pastTrig = 1;
    RF_ble_pCmdBle5AdvAux->channel = 0xFF;
    RF_ble_pCmdBle5AdvAux->whitening.bOverride = 1;
    RF_ble_pCmdBle5AdvAux->whitening.init = config->frequencyTable[config->frequency].whitening;
    RF_ble_pCmdBle5AdvAux->startTime = 0;
#endif
    RF_ble_pCmdBleAdvNc->pParams->pAdvData = packet;
    RF_ble_pCmdBleAdvNc->startTrigger.triggerType  = triggerType;
    RF_ble_pCmdBleAdvNc->startTrigger.pastTrig = 1;
    RF_ble_pCmdBleAdvNc->channel = 0xFF;
    RF_ble_pCmdBleAdvNc->whitening.bOverride = 1;
    RF_ble_pCmdBleAdvNc->whitening.init = config->frequencyTable[config->frequency].whitening;
    RF_ble_pCmdBleAdvNc->startTime = 0;
#endif

    currentDataEntry = (rfc_dataEntryGeneral_t*)&txDataEntryBuffer;
    currentDataEntry->length = config->payloadLength;
    pPacket = &currentDataEntry->data;

#if (defined __CC1310_LAUNCHXL_BOARD_H__)

    /* Modify Setup command and TX Power depending on using Range Extender or not
     * Using CC1310 + CC1190 can only be done for the following PHYs:
     * fsk (50 kbps, 2-GFSK)
     * lrm (Legacy Long Range)
     * sl_lr (SimpleLink Long Range) */
    if (config->rangeExtender == RangeExtender_Dis)
    {
        /* Settings used for the CC1310 LAUNCHXL */
        RF_pCmdPropRadioDivSetup_fsk->txPower   = RF_TxPowerTable_findValue((RF_TxPowerTable_Entry *)PROP_RF_txPowerTable, 14).rawValue;
        RF_pCmdPropRadioDivSetup_lrm->txPower   = RF_TxPowerTable_findValue((RF_TxPowerTable_Entry *)PROP_RF_txPowerTable, 14).rawValue;
        RF_pCmdPropRadioDivSetup_sl_lr->txPower = RF_TxPowerTable_findValue((RF_TxPowerTable_Entry *)PROP_RF_txPowerTable, 14).rawValue;
        {
            uint8_t i = 0;
            do
            {
                if ((pOverrides_fsk[i] & 0x0000FFFF) ==  0x000088A3)
                {
                    pOverrides_fsk[i] = (uint32_t)0x00FB88A3;
                }
            } while ((pOverrides_fsk[i++] != 0xFFFFFFFF));

            i = 0;
            do
            {
                if ((pOverrides_lrm[i] & 0x0000FFFF) ==  0x000088A3)
                {
                    pOverrides_lrm[i] = (uint32_t)0x00FB88A3;
                }
            } while ((pOverrides_lrm[i++] != 0xFFFFFFFF));

            i = 0;
            do
            {
                if ((pOverrides_sl_lr[i] & 0x0000FFFF) ==  0x000088A3)
                {
                    pOverrides_sl_lr[i] = (uint32_t)0x00FB88A3;
                }
            } while ((pOverrides_sl_lr[i++] != 0xFFFFFFFF));
        }
    }
    else
    {
        /* Settings used for the CC1310 CC1190 LAUNCHXL */
        if(config->frequencyTable[config->frequency].frequency == CENTER_FREQ_EU) // 868 MHz
        {
            RF_pCmdPropRadioDivSetup_fsk->txPower   = RF_TxPowerTable_findValue((RF_TxPowerTable_Entry *)PROP_RF_txPowerTableREEU, 25).rawValue;
            RF_pCmdPropRadioDivSetup_lrm->txPower   = RF_TxPowerTable_findValue((RF_TxPowerTable_Entry *)PROP_RF_txPowerTableREEU, 25).rawValue;
            RF_pCmdPropRadioDivSetup_sl_lr->txPower = RF_TxPowerTable_findValue((RF_TxPowerTable_Entry *)PROP_RF_txPowerTableREEU, 25).rawValue;
            {
                uint8_t i = 0;
                do
                {
                    if ((pOverrides_fsk[i] & 0x0000FFFF) ==  0x000088A3)
                    {
                        pOverrides_fsk[i] = (uint32_t)0x000188A3;
                    }
                } while ((pOverrides_fsk[i++] != 0xFFFFFFFF));

                i = 0;
                do
                {
                    if ((pOverrides_lrm[i] & 0x0000FFFF) ==  0x000088A3)
                    {
                        pOverrides_lrm[i] = (uint32_t)0x000188A3;
                    }
                } while ((pOverrides_lrm[i++] != 0xFFFFFFFF));

                i = 0;
                do
                {
                    if ((pOverrides_sl_lr[i] & 0x0000FFFF) ==  0x000088A3)
                    {
                        pOverrides_sl_lr[i] = (uint32_t)0x000188A3;
                    }
                } while ((pOverrides_sl_lr[i++] != 0xFFFFFFFF));
            }
        }
        else if(config->frequencyTable[config->frequency].frequency == CENTER_FREQ_US) // 915 MHz
        {
            RF_pCmdPropRadioDivSetup_fsk->txPower   = RF_TxPowerTable_findValue((RF_TxPowerTable_Entry *)PROP_RF_txPowerTableREUS, 25).rawValue;
            RF_pCmdPropRadioDivSetup_lrm->txPower   = RF_TxPowerTable_findValue((RF_TxPowerTable_Entry *)PROP_RF_txPowerTableREUS, 25).rawValue;
            RF_pCmdPropRadioDivSetup_sl_lr->txPower = RF_TxPowerTable_findValue((RF_TxPowerTable_Entry *)PROP_RF_txPowerTableREUS, 25).rawValue;
            {
                uint8_t i = 0;
                do
                {
                    if ((pOverrides_fsk[i] & 0x0000FFFF) ==  0x000088A3)
                    {
                        pOverrides_fsk[i] = (uint32_t)0x000388A3;
                    }
                } while ((pOverrides_fsk[i++] != 0xFFFFFFFF));

                i = 0;
                do
                {
                    if ((pOverrides_lrm[i] & 0x0000FFFF) ==  0x000088A3)
                    {
                        pOverrides_lrm[i] = (uint32_t)0x000388A3;
                    }
                } while ((pOverrides_lrm[i++] != 0xFFFFFFFF));

                i = 0;
                do
                {
                    if ((pOverrides_sl_lr[i] & 0x0000FFFF) ==  0x000088A3)
                    {
                        pOverrides_sl_lr[i] = (uint32_t)0x000388A3;
                    }
                } while ((pOverrides_sl_lr[i++] != 0xFFFFFFFF));
            }
        }
    }
#endif

    /* Request access to the radio based on test case*/
    switch (config->rfSetup)
    {
        case RfSetup_Custom:
            rfHandle = RF_open(&rfObject, &RF_prop, (RF_RadioSetup*)&RF_cmdPropRadioDivSetup, &rfParams);
            packetInterval = (uint32_t)(RF_convertMsToRatTicks(PKT_INTERVAL_MS_CUSTOM)); // Set packet interval to 60 ms
            cmdTxPower     = RF_cmdPropRadioDivSetup.txPower;
            break;

        case RfSetup_Fsk:
#if !(defined __CC2650DK_7ID_BOARD_H__)      && !(defined __CC2650_LAUNCHXL_BOARD_H__)   && \
    !(defined __CC2640R2_LAUNCHXL_BOARD_H__) && !(defined __CC26X2R1_LAUNCHXL_BOARD_H__)
            RF_pCmdPropRadioDivSetup_fsk->centerFreq = config->frequencyTable[config->frequency].frequency;
            rfHandle = RF_open(&rfObject, RF_pProp_fsk, (RF_RadioSetup*)RF_pCmdPropRadioDivSetup_fsk, &rfParams);
            cmdTxPower     = RF_pCmdPropRadioDivSetup_fsk->txPower;
#else
            rfHandle = RF_open(&rfObject, RF_pProp_2_4G_fsk, (RF_RadioSetup*)RF_pCmdPropRadioSetup_2_4G_fsk, &rfParams);
            cmdTxPower = RF_pCmdPropRadioSetup_2_4G_fsk->txPower;
#endif
            packetInterval = (uint32_t)(RF_convertMsToRatTicks(PKT_INTERVAL_MS_2GFSK)); // Set packet interval to 60 ms
            break;

#if !(defined __CC2650DK_7ID_BOARD_H__)      && !(defined __CC2650_LAUNCHXL_BOARD_H__)   && \
    !(defined __CC2640R2_LAUNCHXL_BOARD_H__) && !(defined __CC26X2R1_LAUNCHXL_BOARD_H__)
        case RfSetup_Sl_lr:
            RF_pCmdPropRadioDivSetup_sl_lr->centerFreq = config->frequencyTable[config->frequency].frequency;
            rfHandle = RF_open(&rfObject, RF_pProp_sl_lr, (RF_RadioSetup*)RF_pCmdPropRadioDivSetup_sl_lr, &rfParams);
            packetInterval = (uint32_t)(RF_convertMsToRatTicks(PKT_INTERVAL_MS_SLR)); // Set packet interval to 80 ms
            cmdTxPower     = RF_pCmdPropRadioDivSetup_sl_lr->txPower;
            break;

#if  !(defined __CC1352R1_LAUNCHXL_BOARD_H__) && !(defined __CC1352P1_LAUNCHXL_BOARD_H__) && \
     !(defined __CC1312R1_LAUNCHXL_BOARD_H__) && !(defined __CC26X2R1_LAUNCHXL_BOARD_H__)
        case RfSetup_Lrm:
            RF_pCmdPropRadioDivSetup_lrm->centerFreq = config->frequencyTable[config->frequency].frequency;
            rfHandle = RF_open(&rfObject, RF_pProp_lrm, (RF_RadioSetup*)RF_pCmdPropRadioDivSetup_lrm, &rfParams);
            packetInterval = (uint32_t)(RF_convertMsToRatTicks(PKT_INTERVAL_MS_LRM)); // Set packet interval to 500 ms
            cmdTxPower     = RF_pCmdPropRadioDivSetup_lrm->txPower;
            break;
#if !(defined __CC1350_LAUNCHXL_433_BOARD_H__)
        case RfSetup_Ook:
            RF_pCmdPropRadioDivSetup_ook->centerFreq = config->frequencyTable[config->frequency].frequency;
            rfHandle = RF_open(&rfObject, RF_pProp_ook, (RF_RadioSetup*)RF_pCmdPropRadioDivSetup_ook, &rfParams);
            packetInterval = (uint32_t)(RF_convertMsToRatTicks(PKT_INTERVAL_MS_OOK)); // Set packet interval to 100 ms
            cmdTxPower     = RF_pCmdPropRadioDivSetup_ook->txPower;
            break;

        case RfSetup_Hsm:
            rfHandle = RF_open(&rfObject, RF_pProp_hsm, (RF_RadioSetup*)RF_pCmdRadioSetup_hsm, &rfParams);
            packetInterval = (uint32_t)(RF_convertMsToRatTicks(PKT_INTERVAL_MS_HSM)); // Set packet interval to 50 ms
            cmdTxPower     = RF_pCmdRadioSetup_hsm->txPower;
            break;
#endif
#endif
#endif

#if (defined __CC2650DK_7ID_BOARD_H__)        || (defined __CC2650_LAUNCHXL_BOARD_H__)   || \
    (defined __CC2640R2_LAUNCHXL_BOARD_H__)   || (defined __CC1350_LAUNCHXL_BOARD_H__)   || \
    (defined __CC1350_LAUNCHXL_433_BOARD_H__) || (defined __CC1350STK_BOARD_H__)         || \
    (defined __CC26X2R1_LAUNCHXL_BOARD_H__)   || (defined __CC1352R1_LAUNCHXL_BOARD_H__) || \
    (defined __CC1352P1_LAUNCHXL_BOARD_H__)
        case RfSetup_Ble:
#if (defined __CC2640R2_LAUNCHXL_BOARD_H__) || (defined __CC1352R1_LAUNCHXL_BOARD_H__) || \
    (defined __CC1352P1_LAUNCHXL_BOARD_H__) || (defined __CC26X2R1_LAUNCHXL_BOARD_H__)
        case RfSetup_Ble5:
#endif
            rfHandle = RF_open(&rfObject, RF_pModeBle, (RF_RadioSetup*)RF_ble_pCmdRadioSetup, &rfParams);
            packetInterval = (uint32_t)(RF_convertMsToRatTicks(PKT_INTERVAL_MS_BLE)); // Set packet interval to 100 ms
            cmdTxPower     = RF_ble_pCmdRadioSetup->txPower;
            break;
#endif
        default:

            break;
    }

    /* Set the packet interval for display purposes */
    if(config->intervalMode == IntervalMode_Yes)
    {
        txMetrics.packetIntervalMs = RF_convertRatTicksToMs(packetInterval);
    }
    else
    {
        // packets sent back-to-back
        txMetrics.packetIntervalMs = 0;
    }

    /* Determine the transmission power in dBm */
#if (defined __CC1310_LAUNCHXL_BOARD_H__)   || (defined __CC1350_LAUNCHXL_BOARD_H__)     || \
    (defined __CC1350STK_BOARD_H__)         || (defined __CC1350_LAUNCHXL_433_BOARD_H__) || \
    (defined __CC1312R1_LAUNCHXL_BOARD_H__) || (defined __CC1352R1_LAUNCHXL_BOARD_H__)   || \
    (defined __CC1352P1_LAUNCHXL_BOARD_H__)

    if(cmdTxPower != RF_TX20_ENABLED)
    {
        /* Default PA mode */
        rfPowerTable = (RF_TxPowerTable_Entry *)PROP_RF_txPowerTable;
        rfPowerTableSize = PROP_RF_txPowerTableSize;

#if (defined __CC1310_LAUNCHXL_BOARD_H__)
        /* Overwrite table pointer if range extender is enabled */
        if((config->rangeExtender == RangeExtender_En) &&
           (config->frequencyTable[config->frequency].frequency == CENTER_FREQ_EU))
        {
           rfPowerTable = (RF_TxPowerTable_Entry *)PROP_RF_txPowerTableREEU;
           rfPowerTableSize = PROP_RF_txPowerTableREEUSize;
        }
        else if((config->rangeExtender == RangeExtender_En) &&
                (config->frequencyTable[config->frequency].frequency == CENTER_FREQ_US))
        {
            rfPowerTable = (RF_TxPowerTable_Entry *)PROP_RF_txPowerTableREUS;
            rfPowerTableSize = PROP_RF_txPowerTableREUSSize;
        }
#endif
#if (defined __CC1350_LAUNCHXL_BOARD_H__)   || (defined __CC1350_LAUNCHXL_433_BOARD_H__) || \
    (defined __CC1350STK_BOARD_H__)         || (defined __CC1352R1_LAUNCHXL_BOARD_H__)   || \
    (defined __CC1352P1_LAUNCHXL_BOARD_H__)
        /* Overwrite table pointer in BLE mode */
        if((config->rfSetup == RfSetup_Ble) 
#if (defined __CC1352R1_LAUNCHXL_BOARD_H__) || (defined __CC1352P1_LAUNCHXL_BOARD_H__)    
        || (config->rfSetup == RfSetup_Ble5)
#endif
          )
        {
            rfPowerTable = (RF_TxPowerTable_Entry *)PROP_RF_txPowerTableBle;
            rfPowerTableSize = PROP_RF_txPowerTableBleSize;
        }
#endif
    }
    else
    {
#if (defined __CC1352P1_LAUNCHXL_BOARD_H__)
        if((config->rfSetup == RfSetup_Ble) || (config->rfSetup == RfSetup_Ble5))
        {
            /* High PA mode on CC1352P2 (BLE) */
            rfPowerTable = (RF_TxPowerTable_Entry *)PROP_RF_txPowerTableBleHpa;
            rfPowerTableSize = PROP_RF_txPowerTableBleHpaSize;
        }
        else
        {
            /* High PA mode on CC1352P1 (Sub-1GHz) */
            rfPowerTable = (RF_TxPowerTable_Entry *)PROP_RF_txPowerTableHpa;
            rfPowerTableSize = PROP_RF_txPowerTableHpaSize;
        }
#endif
    }
    
    /* 
     * Exceptions for 
     *   1. BLE: Tx power is set to the second highest entry when boost mode 
     *      is turned off (CCFG_FORCE_VDDR_HH = 0)
     *   2. High Speed Mode: The Tx power is set to the highest entry in the 
     *      power table when boost mode is turned on (CCFG_FORCE_VDDR_HH = 1)
     *      or the second highest entry when boost is turned off
     *      (CCFG_FORCE_VDDR_HH = 0)
     */
    if(rfPowerTable == NULL)
    {
        txMetrics.transmitPowerDbm = -128;
    }
    else
    {
        RF_TxPowerTable_Value currValue = RF_getTxPower(rfHandle);
        txMetrics.transmitPowerDbm = RF_TxPowerTable_findPowerLevel(rfPowerTable, currValue);
    
    //if CCFG_FORCE_VDDR_HH is not set max power cannot be achieved in Sub-1GHz
    //mode
#if (CCFG_FORCE_VDDR_HH != 0x1)
        if((currValue.paType == RF_TxPowerTable_DefaultPA) && 
            (txMetrics.transmitPowerDbm == rfPowerTable[rfPowerTableSize-2].power)
#if (defined __CC1350_LAUNCHXL_BOARD_H__)   || (defined __CC1350_LAUNCHXL_433_BOARD_H__) || \
    (defined __CC1350STK_BOARD_H__)         || (defined __CC1352R1_LAUNCHXL_BOARD_H__)   || \
    (defined __CC1352P1_LAUNCHXL_BOARD_H__)
            && (config->rfSetup != RfSetup_Ble)
#if (defined __CC1352R1_LAUNCHXL_BOARD_H__) || (defined __CC1352P1_LAUNCHXL_BOARD_H__)
            && (config->rfSetup != RfSetup_Ble5)
#endif
#endif
        )
        {
            txMetrics.transmitPowerDbm = rfPowerTable[rfPowerTableSize-3].power;
        }
#endif

#if !(defined __CC2650DK_7ID_BOARD_H__)      && !(defined __CC2650_LAUNCHXL_BOARD_H__)     && \
    !(defined __CC2640R2_LAUNCHXL_BOARD_H__) && !(defined __CC1350_LAUNCHXL_433_BOARD_H__) && \
    !(defined __CC1352R1_LAUNCHXL_BOARD_H__) && !(defined __CC1352P1_LAUNCHXL_BOARD_H__)   && \
    !(defined __CC26X2R1_LAUNCHXL_BOARD_H__) && !(defined __CC1312R1_LAUNCHXL_BOARD_H__)
        if(config->rfSetup == RfSetup_Hsm)
        {
#if (CCFG_FORCE_VDDR_HH == 0x1)
            txMetrics.transmitPowerDbm = rfPowerTable[rfPowerTableSize-2].power;
#else
            txMetrics.transmitPowerDbm = rfPowerTable[rfPowerTableSize-3].power;
#endif
        }
#endif
    }
    
#else
    uint8_t powerTableIndex;
    for(powerTableIndex = 0; powerTableIndex < rfPowerTableSize; powerTableIndex++)
    {
        if(rfPowerTable[powerTableIndex].txPower == cmdTxPower)
        {
            txMetrics.transmitPowerDbm = rfPowerTable[powerTableIndex].dbm;
            break;
        }
        else
        {
            // TX power configuration not found in the power table
            txMetrics.transmitPowerDbm = -128;
        }
    }
#if (defined __CC2650DK_7ID_BOARD_H__)      || (defined __CC2650_LAUNCHXL_BOARD_H__)   || \
    (defined __CC2640R2_LAUNCHXL_BOARD_H__) || (defined __CC26X2R1_LAUNCHXL_BOARD_H__) 
    if((config->rfSetup == RfSetup_Ble)
#if (defined __CC2640R2_LAUNCHXL_BOARD_H__) || (defined __CC26X2R1_LAUNCHXL_BOARD_H__)
    || (config->rfSetup == RfSetup_Ble5)
#endif
    )
    {
        // If BLE mode is enabled, power is set to 5 dBm
        txMetrics.transmitPowerDbm = 5;
    }
#endif
#endif

    /* Determine the data rate in bits per seconds */
    txMetrics.dataRateBps = config_dataRateTable_Lut[config->rfSetup];

    menu_updateTxMetricScreen(&txMetrics);

    /* Set the frequency */
    if(config->rfSetup == RfSetup_Custom)
    {
        /* Custom settings exported from SmartRf studio shall use the exported frequency */
        RF_runCmd(rfHandle, (RF_Op*)&RF_cmdFs, RF_PriorityNormal, NULL, 0);
    }
#if (defined __CC2650DK_7ID_BOARD_H__)        || (defined __CC2650_LAUNCHXL_BOARD_H__)   || \
    (defined __CC2640R2_LAUNCHXL_BOARD_H__)   || (defined __CC1350_LAUNCHXL_BOARD_H__)   || \
    (defined __CC1350_LAUNCHXL_433_BOARD_H__) || (defined __CC1350STK_BOARD_H__)         || \
    (defined __CC26X2R1_LAUNCHXL_BOARD_H__)   || (defined __CC1352R1_LAUNCHXL_BOARD_H__) || \
    (defined __CC1352P1_LAUNCHXL_BOARD_H__)
    else if((config->rfSetup == RfSetup_Ble)
#if (defined __CC2640R2_LAUNCHXL_BOARD_H__) || (defined __CC1352R1_LAUNCHXL_BOARD_H__) || \
    (defined __CC1352P1_LAUNCHXL_BOARD_H__) || (defined __CC26X2R1_LAUNCHXL_BOARD_H__)
            || (config->rfSetup == RfSetup_Ble5)
#endif
            )
    {
        RF_ble_pCmdFs->frequency = config->frequencyTable[config->frequency].frequency;
        RF_ble_pCmdFs->fractFreq = config->frequencyTable[config->frequency].fractFreq;
        RF_runCmd(rfHandle, (RF_Op*)RF_ble_pCmdFs, RF_PriorityNormal, NULL, 0);
        if(config->intervalMode == IntervalMode_No)
        {
            /* If BLE packets are sent back-to-back the synthesizer is turned
             * off after the first transmission if the advertisement channel is
             * set to 255. The channel must be directly written in the
             * advertisement command itself; it is an offset from 2300 MHz.
             */
            RF_ble_pCmdBleAdvNc->channel = config->frequencyTable[config->frequency].frequency - BLE_BASE_FREQUENCY;
#if (defined __CC2640R2_LAUNCHXL_BOARD_H__) || (defined __CC1352R1_LAUNCHXL_BOARD_H__) || \
    (defined __CC1352P1_LAUNCHXL_BOARD_H__) || (defined __CC26X2R1_LAUNCHXL_BOARD_H__)
            RF_ble_pCmdBle5AdvAux->channel = config->frequencyTable[config->frequency].frequency - BLE_BASE_FREQUENCY;
#endif
        }
    }
#endif
    else
    {
        RF_pCmdFs_preDef->frequency = config->frequencyTable[config->frequency].frequency;
        RF_pCmdFs_preDef->fractFreq = config->frequencyTable[config->frequency].fractFreq;
        RF_runCmd(rfHandle, (RF_Op*)RF_pCmdFs_preDef, RF_PriorityNormal, NULL, 0);
    }

    /* Get current time */
    time = RF_getCurrentTime();

    /* Create packet with incrementing sequence number and random payload */
    pPacket[0] = packet[0] = (uint8_t)(seqNumber >> 8);
    pPacket[1] = packet[1] = (uint8_t)(seqNumber++);
    uint16_t i;
    for (i = 2; i < config->payloadLength; i++)
    {
        pPacket[i] = packet[i] = rand();
    }

    /* Set absolute TX time to utilize automatic power management */
    time += packetInterval;
    RF_cmdPropTx.startTime = time;

    /* Send packet */
    switch (config->rfSetup)
    {
#if !(defined __CC2650DK_7ID_BOARD_H__)      && !(defined __CC2650_LAUNCHXL_BOARD_H__)     && \
    !(defined __CC2640R2_LAUNCHXL_BOARD_H__) && !(defined __CC1350_LAUNCHXL_433_BOARD_H__) && \
    !(defined __CC1352R1_LAUNCHXL_BOARD_H__) && !(defined __CC1352P1_LAUNCHXL_BOARD_H__)   && \
    !(defined __CC26X2R1_LAUNCHXL_BOARD_H__) && !(defined __CC1312R1_LAUNCHXL_BOARD_H__)
        case RfSetup_Hsm:
        {
            RF_pCmdTxHS->startTime = time;
            cmdHandle = RF_postCmd(rfHandle, (RF_Op*)RF_pCmdTxHS, RF_PriorityNormal, &tx_callback, 0);
            break;
        }
#endif

#if (defined __CC2650DK_7ID_BOARD_H__)        || (defined __CC2650_LAUNCHXL_BOARD_H__)   || \
    (defined __CC2640R2_LAUNCHXL_BOARD_H__)   || (defined __CC1350_LAUNCHXL_BOARD_H__)   || \
    (defined __CC1350_LAUNCHXL_433_BOARD_H__) || (defined __CC1350STK_BOARD_H__)         || \
    (defined __CC26X2R1_LAUNCHXL_BOARD_H__)   || (defined __CC1352R1_LAUNCHXL_BOARD_H__) || \
    (defined __CC1352P1_LAUNCHXL_BOARD_H__)
        case RfSetup_Ble:
        {
            RF_ble_pCmdBleAdvNc->startTime = time;
            cmdHandle = RF_postCmd(rfHandle, (RF_Op*)RF_ble_pCmdBleAdvNc, RF_PriorityNormal, &tx_callback, 0);
            break;
        }
#if (defined __CC2640R2_LAUNCHXL_BOARD_H__) || (defined __CC1352R1_LAUNCHXL_BOARD_H__) || \
    (defined __CC1352P1_LAUNCHXL_BOARD_H__) || (defined __CC26X2R1_LAUNCHXL_BOARD_H__)
        case RfSetup_Ble5:
        {
            RF_ble_pCmdBle5AdvAux->startTime = time;
            cmdHandle = RF_postCmd(rfHandle, (RF_Op*)RF_ble_pCmdBle5AdvAux, RF_PriorityNormal, &tx_callback, 0);
            break;
        }
#endif
#endif
        default:
        {
            cmdHandle = RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropTx, RF_PriorityNormal, &tx_callback, 0);
            break;
        }
    }

    while (!bPacketTxDone)
    {
        /* Check, whether a button has been pressed */
        if (menu_isButtonPressed())
        {
            /* If there is an ongoing Tx command, cancel it */
            (void)RF_cancelCmd(rfHandle, cmdHandle, ABORT_GRACEFUL);
            RF_pendCmd(rfHandle, cmdHandle, (RF_EventCmdCancelled | RF_EventCmdStopped | RF_EventCmdAborted));
            RF_close(rfHandle);

            /* Do a final update to indicate #packets sent*/
            menu_updateTxScreen(packetCount);

            bPacketTxDone = false;
            packetCount = 0;
            seqNumber = 0;
            return TestResult_Aborted;
        }
        else if(packetCount != lastpacketCount)
        {
            /* Update the display */
            menu_updateTxScreen(packetCount);
            lastpacketCount = packetCount;
        }
    }

    if(packetCount == config->packetCount)
    {
        /* Do a final update to indicate all packets were sent*/
        menu_updateTxScreen(packetCount);
    }

    bPacketTxDone = false;
    packetCount = 0;
    seqNumber = 0;
    RF_close(rfHandle);
    return TestResult_Finished;
}

void tx_callback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
    if(e & RF_EventLastCmdDone)
    {
        /* Increment the packet counter */
        packetCount++;

        if(packetCount < localConfig.packetCount)
        {
            /* Increment the sequence number for the next packet but keep
             * the same data */
            pPacket[0] = packet[0] = (uint8_t)(seqNumber >> 8);
            pPacket[1] = packet[1] = (uint8_t)(seqNumber++);

            /* Set absolute TX time to utilize automatic power management */
            time += packetInterval;
            RF_cmdPropTx.startTime = time;

            /* Send packet */
            switch (localConfig.rfSetup)
            {
        #if !(defined __CC2650DK_7ID_BOARD_H__)      && !(defined __CC2650_LAUNCHXL_BOARD_H__)     && \
            !(defined __CC2640R2_LAUNCHXL_BOARD_H__) && !(defined __CC1350_LAUNCHXL_433_BOARD_H__) && \
            !(defined __CC1352R1_LAUNCHXL_BOARD_H__) && !(defined __CC1352P1_LAUNCHXL_BOARD_H__)   && \
            !(defined __CC26X2R1_LAUNCHXL_BOARD_H__) && !(defined __CC1312R1_LAUNCHXL_BOARD_H__)
                case RfSetup_Hsm:
                {
                    RF_pCmdTxHS->startTime = time;
                    cmdHandle = RF_postCmd(rfHandle, (RF_Op*)RF_pCmdTxHS, RF_PriorityNormal, &tx_callback, 0);
                    break;
                }
        #endif

        #if (defined __CC2650DK_7ID_BOARD_H__)        || (defined __CC2650_LAUNCHXL_BOARD_H__)   || \
            (defined __CC2640R2_LAUNCHXL_BOARD_H__)   || (defined __CC1350_LAUNCHXL_BOARD_H__)   || \
            (defined __CC1350_LAUNCHXL_433_BOARD_H__) || (defined __CC1350STK_BOARD_H__)         || \
            (defined __CC26X2R1_LAUNCHXL_BOARD_H__)   || (defined __CC1352R1_LAUNCHXL_BOARD_H__) || \
            (defined __CC1352P1_LAUNCHXL_BOARD_H__)
                case RfSetup_Ble:
                {
                    RF_ble_pCmdBleAdvNc->startTime = time;
                    cmdHandle = RF_postCmd(rfHandle, (RF_Op*)RF_ble_pCmdBleAdvNc, RF_PriorityNormal, &tx_callback, 0);
                    break;
                }
        #if (defined __CC2640R2_LAUNCHXL_BOARD_H__) || (defined __CC1352R1_LAUNCHXL_BOARD_H__) || \
            (defined __CC1352P1_LAUNCHXL_BOARD_H__) || (defined __CC26X2R1_LAUNCHXL_BOARD_H__)
                case RfSetup_Ble5:
                {
                    RF_ble_pCmdBle5AdvAux->startTime = time;
                    cmdHandle = RF_postCmd(rfHandle, (RF_Op*)RF_ble_pCmdBle5AdvAux, RF_PriorityNormal, &tx_callback, 0);
                    break;
                }
        #endif
        #endif
                default:
                {
                    cmdHandle = RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropTx, RF_PriorityNormal, &tx_callback, 0);
                    break;
                }
            }
        }
        else
        {
            bPacketTxDone = true;
        }
    }
}
