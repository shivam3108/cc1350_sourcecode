/******************************************************************************

 @file oad.c

 @brief Over the Air Download for use with the BLE OAD example

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

/******************************************************************************
 Includes
 *****************************************************************************/
#if defined(FEATURE_BLE_OAD)
#include <string.h>
#include <stdint.h>
#include <sys_ctrl.h>
#include <ti/mw/extflash/ExtFlash.h>
#include "common/native_oad/ext_flash_layout.h"

/******************************************************************************
 Constants and definitions
 *****************************************************************************/

/* dummy image metadata */
#if defined (__IAR_SYSTEMS_ICC__)
#pragma location=".checksum"
/* 4 bytes for CRC and CRC Shadow. */
const uint8_t _chksum[4] =
{
    0xFF, 0xFF, 0xFF, 0xFF
};
/* 12 byte for image header immediately following CRC/CRC Shadow. */
#pragma location="IMAGE_HEADER"
const uint8_t _imgHdr[12] =
{
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF
};
#elif defined __TI_COMPILER_VERSION__
#pragma DATA_SECTION(_imgHdr, ".imgHdr")
#pragma RETAIN(_imgHdr)
/*
 For CCS, this is the first 16 bytes at a 0 byte offset from the start of the
 OAD Image.
 */
const uint8_t _imgHdr[16] =
    { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF };
#endif

/******************************************************************************
 External variables
 *****************************************************************************/

/******************************************************************************
 Public variables
 *****************************************************************************/

/******************************************************************************
 Local variables
 *****************************************************************************/

ExtImageInfo_t imgInfo;

/******************************************************************************
 Local function prototypes
 *****************************************************************************/

/******************************************************************************
 Public Functions
 *****************************************************************************/

/*!
 Marks the image in external flash as ready to run

 Public function defined in oad.h
 */
void OAD_markSwitch(void)
{
    if(!ExtFlash_open())
    {
        return;
    }

    /*
     Read the second image metadata from external flash. We assume that the
     user has loaded a full image into the second image slot. The factory
     image concept has not been implemented in the BLE OAD example as of this
     writing.
     */
    ExtFlash_read(EFL_IMAGE_INFO_ADDR_BLE, sizeof(ExtImageInfo_t),
                  (uint8_t *) &imgInfo);

    if((imgInfo.crc[0] == imgInfo.crc[1]) && 
            (imgInfo.imgType == EFL_OAD_IMG_TYPE_STACK))
    {
        /*
         Mark the image as ready to run if the crc is valid and pointing to
         the second image slot, then reset to let the BIM load the new image.
         */
        ExtFlash_erase(EFL_IMAGE_INFO_ADDR_BLE, EFL_PAGE_SIZE);
        imgInfo.status = 0xFF;
        ExtFlash_write(EFL_IMAGE_INFO_ADDR_BLE, sizeof(ExtImageInfo_t),
                       (uint8_t *) &imgInfo);

        SysCtrlSystemReset();
    }

    /*
     We did not find a valid image, close the driver and return.
     */
    ExtFlash_close();
    return;
}

/******************************************************************************
 Local Functions
 *****************************************************************************/

#endif /* FEATURE_BLE_OAD */
