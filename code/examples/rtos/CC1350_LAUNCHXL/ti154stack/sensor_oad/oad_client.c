/******************************************************************************

 @file oad_client.c

 @brief OAD Client

 Group: WCS LPC
 Target Device: CC13xx

 ******************************************************************************
 
 Copyright (c) 2016-2018, Texas Instruments Incorporated
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 *  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

 *  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 *  Neither the name of Texas Instruments Incorporated nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************
 Release Name: simplelink_cc13x0_sdk_2_10_00_
 Release Date: 2018-04-09 00:04:23
 *****************************************************************************/
#ifndef OAD_CLIENT_H
#define OAD_CLIENT_H

#ifdef FEATURE_NATIVE_OAD
/******************************************************************************
 Includes
 *****************************************************************************/
#include <string.h>
#include <stdint.h>

#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Semaphore.h>

#include "util.h"
#include "jdllc.h"
#include "ssf.h"
#include "sensor.h"
#include "board_lcd.h"
#include "timer.h"
#include "mac_timer.h"
#include "mac_spec.h"

#include <sensor_oad/oad_client.h>
#include <common/native_oad/oad_protocol.h>
#include <common/native_oad/oad_storage.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/sys_ctrl.h)
#include DeviceFamily_constructPath(driverlib/flash.h)

/******************************************************************************
 Constants and definitions
 *****************************************************************************/

#define FW_VERSION "v1.0"

/* symbol durations for sub-1g only */
#if (CONFIG_PHY_ID == APIMAC_STD_US_915_PHY_1)
    #define SYMBOL_DURATION         (SYMBOL_DURATION_50_kbps)  //us
#elif (CONFIG_PHY_ID == APIMAC_GENERIC_US_915_PHY_132)
    #define SYMBOL_DURATION         (SYMBOL_DURATION_200_kbps) //us
#elif (CONFIG_PHY_ID == APIMAC_GENERIC_US_LRM_915_PHY_129)
    #define SYMBOL_DURATION         (SYMBOL_DURATION_LRM)      //us
#else
    #define SYMBOL_DURATION         (SYMBOL_DURATION_50_kbps)  //us
#endif

#define BEACON_INTERVAL             ((((0x01) << (CONFIG_BEACON_ORDER)) * \
                                      (SYMBOL_DURATION) * (BASE_SUPER_FRAME_DURATION)) / (1000)) // ms

#if (CONFIG_SUPERFRAME_ORDER == 15)
#define OAD_BLOCK_REQ_RATE          200
#define OAD_BLOCK_REQ_POLL_DELAY    50
#define OAD_MAX_TIMEOUTS            3
#define OAD_MAX_RETRIES             3
#define OAD_BLOCK_AUTO_RESUME_DELAY 5000
#else
#define OAD_BLOCK_REQ_RATE          ((BEACON_INTERVAL) - 100)
#define OAD_BLOCK_REQ_POLL_DELAY    ((BEACON_INTERVAL) - 400)
#define OAD_MAX_TIMEOUTS            3
#define OAD_MAX_RETRIES             3
#define OAD_BLOCK_AUTO_RESUME_DELAY ((BEACON_INTERVAL) * 5)
#endif

/*!
 OAD block variables.
 */
static uint16_t oadBNumBlocks = 0;
static uint16_t oadBlock = 0;
static ApiMac_sAddr_t oadServerAddr = {0};
static bool oadInProgress = false;
static uint8_t oadImgId = 0;
static uint8_t oadRetries = 0;
static uint8_t oadTimeouts = 0;

uint8_t oadImgHdr[16];

OADClient_Params_t oadClientParams;

static Clock_Struct oadClkStruct;
static Clock_Handle oadClkHandle;

static uint32_t defualtPollInterval;

/******************************************************************************
 Local function prototypes
 *****************************************************************************/

static void oadClockInitialize(void);
static void oadClockCallback(UArg a0);
static void oadClockSet(uint32_t oadTimeout);
static void displayOadBlockUpdate(uint16_t oadBlock, uint16_t oadBNumBlocks, uint8_t retries);
static void displayOadStatusUpdate(OADStorage_Status_t status);
static void oadFwVersionReqCb(void* pSrcAddr);
static void oadImgIdentifyReqCb(void* pSrcAddr, uint8_t imgId, uint8_t *imgMetaData);
static void oadBlockRspCb(void* pSrcAddr, uint8_t imgId, uint16_t blockNum, uint8_t *blkData);
void* oadRadioAccessAllocMsg(uint32_t msgLen);
static OADProtocol_Status_t oadRadioAccessPacketSend(void* pDstAddr, uint8_t *pMsg, uint32_t msgLen);

/******************************************************************************
 Callback tables
 *****************************************************************************/

static OADProtocol_RadioAccessFxns_t  oadRadioAccessFxns =
    {
      oadRadioAccessAllocMsg,
      oadRadioAccessPacketSend
    };

static OADProtocol_MsgCBs_t oadMsgCallbacks =
    {
      /*! Incoming FW Req */
      oadFwVersionReqCb,
      /*! Incoming FW Version Rsp */
      NULL,
      /*! Incoming Image Identify Req */
      oadImgIdentifyReqCb,
      /*! Incoming Image Identify Rsp */
      NULL,
      /*! Incoming OAD Block Req */
      NULL,
      /*! Incoming OAD Block Rsp */
      oadBlockRspCb
    };

/******************************************************************************
 Public Functions
 *****************************************************************************/

/*!
 Initialize this application.

 Public function defined in sensor.h
 */
void OADClient_open(OADClient_Params_t *params)
{
    OADProtocol_Params_t OADProtocol_params;
    
    memcpy(&oadClientParams, params, sizeof(OADClient_Params_t));    

    OADProtocol_Params_init(&OADProtocol_params);
    OADProtocol_params.pRadioAccessFxns = &oadRadioAccessFxns;
    OADProtocol_params.pProtocolMsgCallbacks = &oadMsgCallbacks;

    OADProtocol_open(&OADProtocol_params);

    oadClockInitialize();
}

/*!
 Application task processing.

 Public function defined in sensor.h
 */
void OADClient_processEvent(uint16_t *pEvent)
{
    /* Is it time to send the next sensor data message? */
    if(*pEvent & SENSOR_OAD_TIMEOUT_EVT)
    {
        static int32_t lastBlock = -1;
        static bool oadComplete = false;

        if(oadComplete)
        {
            /* reset to BIM */
            SysCtrlSystemReset();

            /* Clear complete flag in case reset did not work */
            oadComplete = false;

        }

        if(oadInProgress)
        {
            if(oadBlock == lastBlock)
            {
                if(oadTimeouts++ > OAD_MAX_TIMEOUTS)
                {
                    /*
                     * allow 3 retries before aborting
                     */
                    if(oadRetries++ > OAD_MAX_RETRIES)
                    {
                        /*abort OAD with auto resume */
                        OADClient_abort(true);
                    }
                    else
                    {
                        /* reduce the lastBlock by 1 to force a re-request */
                        lastBlock -= 1;
                        oadTimeouts = 0;
                    }
                }
                else
                {
                    /*
                     * req timed out, set timer again
                     * */
                    oadClockSet(OAD_BLOCK_REQ_RATE);
                    Ssf_setPollClock(OAD_BLOCK_REQ_POLL_DELAY);
                }
            }
        }

        if(oadInProgress)
        {
            if( (oadBlock < oadBNumBlocks) && (oadBlock != lastBlock) )
            {
                if(oadBlock == 0)
                {
                    /* Only store image header and image ID on first block */
                    Ssf_oadInfoUpdate((uint16_t*) &lastBlock, oadImgHdr, &oadImgId, &oadServerAddr);
                }
                else
                {
                    Ssf_oadInfoUpdate((uint16_t*) &lastBlock, 0, 0, 0);
                }
                lastBlock = oadBlock;

                /* Send Block request specifying number of blocks per multi
                 * blocks. The multi block hard coded to 1 here, but it is
                 * intended that an advanced user would use information from
                 * oadRetries and Sensor_msgStats to implement a sliding window
                 * algorithm
                 */
                OADProtocol_sendOadImgBlockReq(&oadServerAddr, oadImgId, oadBlock, 1);

                /* update display */
                displayOadBlockUpdate(oadBlock, oadBNumBlocks, oadRetries);

                /*
                 * Send poll request to get rsp in OAD_BLOCK_REQ_POLL_DELAY ms
                 * to flush the block response from OAD servers MAC Queue
                 */
                Ssf_setPollClock(OAD_BLOCK_REQ_POLL_DELAY);

                /*
                 * set timer to time out in OAD_BLOCK_REQ_RATE ms to request
                 * or re-request block
                 * */
                oadClockSet(OAD_BLOCK_REQ_RATE);
            }
            else if(oadBlock >= oadBNumBlocks)
            {
                OADStorage_Status_t status;

                /*
                 * Check that CRC is correct and mark the image as new
                 * image to be booted in to by BIM on next reset
                 */
                status = OADStorage_imgFinalise();

                /* Display result */
                displayOadStatusUpdate(status);

                /* Stop any further OAD message processing */
                oadInProgress = false;
                lastBlock = -1;
                oadBlock = 0;

                /* set block count to 0 in NV */
                Ssf_oadInfoUpdate(&oadBlock, 0, 0, 0);

                /* Set poll rate back to default */
                Jdllc_setPollRate(defualtPollInterval);

                /* If OAD was successful reset the device */
                if(status == OADStorage_Status_Success)
                {
                    oadComplete = true;

                    /*
                     * Set time 1 more time to reset the device
                     * after the display ha been written to
                     */
                    oadClockSet(OAD_BLOCK_REQ_RATE);
                }
                else
                {
                    //something went wrong abort
                    oadClockSet(0);
                }
            }
        }

        /* Clear the event  */
        Util_clearEvent(pEvent, SENSOR_OAD_TIMEOUT_EVT);
    }
}

/*!
 Abort the OAD.

 Public function defined in sensor.h
 */
void OADClient_abort(bool resume)
{
    if(oadInProgress)
    {
        static uint16_t prevResumeBlock = 0;

        /* Stop OAD timer */
        oadClockSet(0);

        /* Set poll rate back to default */
        Jdllc_setPollRate(defualtPollInterval);

        displayOadStatusUpdate(OADStorage_Aborted);

        /* start timer to auto resume */
        if( resume &&
            (OAD_BLOCK_AUTO_RESUME_DELAY > 0) &&
            (oadBlock != prevResumeBlock) )
        {
            oadClockSet(OAD_BLOCK_AUTO_RESUME_DELAY);
            prevResumeBlock = oadBlock;
            oadRetries = 0;
            oadTimeouts = 0;
        }
        else
        {
            /*abort OAD */
            oadInProgress = false;
            OADStorage_close();
        }
    }
}

/*!
 Resume the OAD.

 Public function defined in sensor.h
 */
void OADClient_resume(uint32_t delay)
{

    Ssf_getOadInfo(&oadBlock, oadImgHdr, &oadImgId, &oadServerAddr);

    if(oadBlock != 0)
    {
        uint32_t page = (oadBlock * (OAD_BLOCK_SIZE)) / EFL_PAGE_SIZE;
        /* Round block number to nearest page boundary in case flash page is
         * corrupted
         */
        oadBlock = (page * EFL_PAGE_SIZE) / (OAD_BLOCK_SIZE);


        oadBNumBlocks = OADStorage_imgIdentifyWrite(oadImgHdr, false);
        if(oadBNumBlocks > 0)
        {
            //erase the page in case it was corrupted during a reset / power cycle
            OADStorage_eraseImgPage(page);
            oadInProgress = true;
            oadClockSet(delay);
        }
    }
}

/******************************************************************************
 Local Functions
 *****************************************************************************/
/*!
 * @brief   OAD timeout handler function.
 *
 * @param   a0 - ignored
 */
static void oadClockCallback(UArg a0)
{
    (void)a0; /* Parameter is not used */

    Util_setEvent(oadClientParams.pEvent, SENSOR_OAD_TIMEOUT_EVT);

    /* Wake up the application thread when it waits for clock event */
    Semaphore_post(oadClientParams.eventSem);
}

static void oadClockInitialize(void)
{
    /* Initialize the timers needed for this application */
    oadClkHandle = Timer_construct(&oadClkStruct,
                                        oadClockCallback,
                                        OAD_BLOCK_REQ_RATE,
                                        0,
                                        false,
                                        0);
}

/*!
 Set the oad clock.

 Public function defined in ssf.h
 */
static void oadClockSet(uint32_t oadTimeout)
{
    /* Stop the Reading timer */
    if(Timer_isActive(&oadClkStruct) == true)
    {
        Timer_stop(&oadClkStruct);
    }

    /* Setup timer */
    if ( oadTimeout )
    {
        Timer_setTimeout(oadClkHandle, oadTimeout);
        Timer_start(&oadClkStruct);
    }
}

/*!
 The application calls this function to indicate oad block.

 Public function defined in ssf.h
 */
static void displayOadBlockUpdate(uint16_t oadBlock, uint16_t oadBNumBlocks, uint8_t retries)
{
    static uint8_t lastRetries;

    LCD_WRITE_STRING_VALUE("OAD Block: ", oadBlock + 1, 10, 3);
    LCD_WRITE_STRING_VALUE("of ", oadBNumBlocks, 10, 3);

    if(lastRetries != retries)
    {
        LCD_WRITE_STRING_VALUE("OAD Retries: ", retries, 10, 3);
    }
}

/*!
 The application calls this function to indicate oad block.

 Public function defined in ssf.h
 */
static void displayOadStatusUpdate(OADStorage_Status_t status)
{
    switch(status)
    {
    case OADStorage_Status_Success:
        LCD_WRITE_STRING("OAD completed successfully", 3);
        break;
    case OADStorage_CrcError:
        LCD_WRITE_STRING("OAD CRC failed", 3);
        break;
    case OADStorage_Aborted:
        LCD_WRITE_STRING("OAD aborted", 3);
        break;
    default:
        LCD_WRITE_STRING("OAD failed", 3);
        break;
    }
}

/*!
 * @brief      FW Version request callback
 */
static void oadFwVersionReqCb(void* pSrcAddr)
{
    char fwVersionStr[OADProtocol_FW_VERSION_STR_LEN] = FW_VERSION;

    //Send response back
    OADProtocol_sendFwVersionRsp(pSrcAddr, fwVersionStr);
}

/*!
 * @brief      OAD image identify request callback
 */
static void oadImgIdentifyReqCb(void* pSrcAddr, uint8_t imgId, uint8_t *imgMetaData)
{
    /* Store the img header incase of resume */
    memcpy(oadImgHdr, imgMetaData, 16);

    /*store the image ID to get blocks for */
    oadImgId = imgId;

    oadBNumBlocks = OADStorage_imgIdentifyWrite(imgMetaData, true);

    oadServerAddr.addrMode = ((ApiMac_sAddr_t*)pSrcAddr)->addrMode;
    if(oadServerAddr.addrMode == ApiMac_addrType_short)
    {
        oadServerAddr.addr.shortAddr = ((ApiMac_sAddr_t*)pSrcAddr)->addr.shortAddr;
    }
    else
    {
        memcpy(oadServerAddr.addr.extAddr, ((ApiMac_sAddr_t*)pSrcAddr)->addr.extAddr,
               (APIMAC_SADDR_EXT_LEN));
    }

    if(oadBNumBlocks)
    {
        oadInProgress = true;
        oadRetries = 0;
        oadTimeouts = 0;
        oadBlock = 0;

        //Save current Poll interval
        defualtPollInterval = Ssf_getPollClock();

        //Send success response back
        OADProtocol_sendOadIdentifyImgRsp(pSrcAddr, 1);

        //Set OAD time out to OAD_BLOCK_REQ_RATE ms
        oadClockSet(OAD_BLOCK_REQ_RATE);
    }
    else
    {
        displayOadStatusUpdate(OADStorage_Rejected);

        //Send fail response back
        OADProtocol_sendOadIdentifyImgRsp(pSrcAddr, 0);
    }
}

/*!
 * @brief      OAD image block response callback
 */
static void oadBlockRspCb(void* pSrcAddr, uint8_t imgId, uint16_t blockNum, uint8_t *blkData)
{
    if( (blockNum == oadBlock) && (oadInProgress) && (oadImgId == imgId))
    {
        oadBlock++;
        oadRetries = 0;
        oadTimeouts = 0;

        OADStorage_imgBlockWrite(blockNum, blkData);

        //set poll rate back to default
        Ssf_setPollClock(defualtPollInterval);
        Jdllc_setPollRate(defualtPollInterval);
    }
}

/*!
 * @brief      Radio access function for OAD module to send messages
 */
void* oadRadioAccessAllocMsg(uint32_t msgLen)
{
    uint8_t *msgBuffer;

    //allocate buffer for CmdId + message
    msgBuffer = Ssf_malloc(msgLen + 1);

    memset(msgBuffer, 0, msgLen + 1);

    return msgBuffer + 1;
}

/*!
 * @brief      Radio access function for OAD module to send messages
 */
static OADProtocol_Status_t oadRadioAccessPacketSend(void* pDstAddr, uint8_t *pMsg, uint32_t msgLen)
{
    OADProtocol_Status_t status = OADProtocol_Failed;
    uint8_t* pMsduPayload;

    //buffer should have been allocated with oadRadioAccessAllocMsg,
    //so 1 byte before the oad msg buffer was allocated for the Smsgs_cmdId
    pMsduPayload = pMsg - 1;
    pMsduPayload[0] = Smsgs_cmdIds_oad;

    if(Sensor_sendMsg(Smsgs_cmdIds_oad, (ApiMac_sAddr_t*) pDstAddr, true,
               msgLen+1 , pMsduPayload) == true)
    {
        status = OADProtocol_Status_Success;
    }

    //free the memory allocated in oadRadioAccessAllocMsg
    Ssf_free(pMsduPayload);

    return status;
}

#endif //FEATURE_NATIVE_OAD

#endif //OAD_CLIENT_H
