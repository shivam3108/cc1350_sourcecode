/******************************************************************************

 @file  bim_main.c

 @brief This module contains the definitions for the main functionality of a
        Boot  Image Manager.

 Group: WCS, BTS
 Target Device: CC13xx

 ******************************************************************************
 
 Copyright (c) 2012-2018, Texas Instruments Incorporated
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
 Release Date: 2018-04-09 00:04:22
 *****************************************************************************/

/* -----------------------------------------------------------------------------
 *                                          Includes
 * -----------------------------------------------------------------------------
 */
#include <driverlib/pwr_ctrl.h>
#include "oad_target.h"

#include <inc/hw_device.h>
#include <stdbool.h>
#include <stdint.h>
#include <driverlib/flash.h>
#include <driverlib/sys_ctrl.h>
#include <driverlib/aon_event.h>
#include <driverlib/osc.h>
/* -----------------------------------------------------------------------------
 *                                          Constants
 * -----------------------------------------------------------------------------
 */

#define BIM_IMG_A_PAGE        OAD_IMG_A_PAGE
#define BIM_IMG_A_OSET        OAD_IMG_A_OSET
#define BIM_IMG_A_AREA        OAD_IMG_A_AREA

#define BIM_IMG_B_PAGE        OAD_IMG_B_PAGE
#define BIM_IMG_B_OSET        OAD_IMG_B_OSET
#define BIM_IMG_B_AREA        OAD_IMG_B_AREA

#define BIM_CRC_OSET          OAD_IMG_CRC_OSET
#define BIM_HDR_OSET          OAD_IMG_HDR_OSET

#define BIM_VER_OSET          4

#define HAL_FLASH_WORD_SIZE   4
#define HAL_FLASH_PAGE_SIZE   4096

/* -----------------------------------------------------------------------------
 *                                          Externs
 * -----------------------------------------------------------------------------
 */

void trimDevice(void);

/* -----------------------------------------------------------------------------
 *                                          Typedefs
 * -----------------------------------------------------------------------------
 */
/*
typedef struct {
  // User-defined Image Version Number - default logic uses simple a '<'
  // comparison to start an OAD.
  uint16_t ver;
  uint16_t len;        // Image length in 4-byte blocks (i.e. HAL_FLASH_WORD_SIZE blocks).
  uint8_t  uid[4];     // User-defined Image Identification bytes.
  uint8_t  res[4];     // Reserved space for future use.
} OADTarget_ImgHdr_t;
*/
/* -----------------------------------------------------------------------------
 *                                       Global Variables
 * -----------------------------------------------------------------------------
 */

/* -----------------------------------------------------------------------------
 *                                       Local Variables
 * -----------------------------------------------------------------------------
 */

uint8_t pgBuf[HAL_FLASH_PAGE_SIZE];


static uint16_t crc16(uint16_t crc, uint8_t val);

/*******************************************************************************
 * @fn     Bim_intWriteWord
 *
 * @brief  Write a word to internal flash.
 *
 * @param  addr - address
 * @param  word - pointer to the word to write
 *
 * @return Zero when successful. Non-zero, otherwise.
 */
static int Bim_intWriteWord(uint_least32_t addr, uint32_t *word)
{
  if (FlashProgram((uint8_t *) word, addr, 4) == FAPI_STATUS_SUCCESS)
  {
    return 0;
  }

  return -1;
}

/*******************************************************************************
 * @fn          crcCalc
 *
 * @brief       Run the CRC16 Polynomial calculation over the image specified.
 *
 * @param       page   - Flash page on which to beginning the CRC calculation.
 *
 * @param       offset - offset of first byte of image within first flash page
 *                       of the image.
 * @return      The CRC16 calculated.
 */
static uint16_t crcCalc(uint8_t page, uint16_t offset)
{
  uint16_t crc = 0;

  // Read first page of the image into the buffer.
  memcpy(pgBuf, (uint8_t *)(page * HAL_FLASH_PAGE_SIZE), HAL_FLASH_PAGE_SIZE);

  const OADTarget_ImgHdr_t *pImgHdr = (const OADTarget_ImgHdr_t *)(pgBuf + offset + BIM_HDR_OSET);

  uint8_t pageBeg = page;
  uint8_t pageEnd = pImgHdr->len / (OAD_FLASH_PAGE_MULT);

  // Find ending offset on last page.
  uint16_t osetEnd = ((pImgHdr->len + (offset / HAL_FLASH_WORD_SIZE)) -
                    (pageEnd * (HAL_FLASH_PAGE_SIZE / HAL_FLASH_WORD_SIZE))) *
                    HAL_FLASH_WORD_SIZE;

  // Set pageEnd to the end page of the OAD range.
  pageEnd += pageBeg;

  // Read over image pages.
  for (uint8_t pageIdx = pageBeg; pageIdx <= pageEnd; pageIdx++)
  {
    // Read over all flash words in a page, excluding the CRC section of the
    // first page and all bytes after the remainder bytes on the last page.
    for (uint16_t oset = (pageIdx == pageBeg ? offset + 4 : 0);
         oset < HAL_FLASH_PAGE_SIZE && (pageIdx < pageEnd || oset < osetEnd);
         oset++)
    {
      crc = crc16(crc, pgBuf[oset]);
    }

    // Read the next page into the buffer.
    if (pageIdx != pageEnd)
    {
      memcpy(pgBuf, (uint8_t *)((pageIdx + 1)*HAL_FLASH_PAGE_SIZE),
             HAL_FLASH_PAGE_SIZE);
    }
  }

  // IAR note explains that poly must be run with value zero for each byte of crc.
  crc = crc16(crc, 0);
  crc = crc16(crc, 0);

  return crc;
}

/*******************************************************************************
 * @fn          crcCheck
 *
 * @brief       Calculate the image CRC and set it ready-to-run if it is good.
 *
 * input parameters
 *
 * @param       page - Flash page on which to beginning the CRC calculation.
 *
 * @param       offset - offset into page at which the image starts.
 *
 * output parameters
 *
 * None.
 *
 * @return      None, but no return from this function if the CRC check is good.
 */
static uint8_t crcCheck(uint8_t page, uint16_t offset, uint16_t *crc)
{
  //HAL_BOARD_INIT();

  // Calculate CRC and compare with original output.
  if (crc[0] == crcCalc(page, offset))
  {
    // Set shadow equal to the original CRC output.
    crc[1] = crc[0];

    // Write CRC & CRC shadow to flash.
    Bim_intWriteWord((page * HAL_FLASH_PAGE_SIZE) + offset + BIM_CRC_OSET,
                     (uint32_t *)crc);

    // Allow branch to application.
    return 1;
  }

  // Image denied!
  return 0;
}

/*******************************************************************************
 * @fn          crc16
 *
 * @brief       Run the CRC16 Polynomial calculation over the byte parameter.
 *
 * input parameters
 *
 * @param       crc - Running CRC calculated so far.
 * @param       val - Value on which to run the CRC16.
 *
 * output parameters
 *
 * None.
 *
 * @return      crc - Updated for the run.
 */
static uint16_t crc16(uint16_t crc, uint8_t val)
{
  const uint16_t poly = 0x1021;
  uint8_t cnt;

  for (cnt = 0; cnt < 8; cnt++, val <<= 1)
  {
    uint8_t msb = (crc & 0x8000) ? 1 : 0;

    crc <<= 1;

    if (val & 0x80)
    {
      crc |= 0x0001;
    }

    if (msb)
    {
      crc ^= poly;
    }
  }

  return crc;
}

/*******************************************************************************
 * @fn          isCrcValid -- documentation
 *
 * @brief       Computes the CRC value of an image and compares it with the CRC
 *              stored in the OAD header.
 *
 * input parameters
 *
 * @param       page - The flash page to start the CRC computation from.
 * @param       offset - The offset value of where the image resides on the
 *                       flash page.
 *
 * output parameters
 *
 * None.
 *
 * @return      True if the computed CRC value matches the CRC value stored in
 *              the OAD header.
 */
bool isCrcValid(uint8_t page, uint32_t offset)
{
    uint16_t crc[2];

    memcpy((uint8_t *)crc,
           (uint32_t *)((page * HAL_FLASH_PAGE_SIZE) + offset + BIM_CRC_OSET), 4);

    // Verify that the CRC exists and the CRC shadow matches
    if ((crc[0] != 0xFFFF) && (crc[0] != 0x0000))
    {
      if (crc[0] == crc[1] || crcCheck(page, offset, crc) )
      {
          return true;
      }
    }

    return false;
}

/*******************************************************************************
 * @fn          isImgBVersionNewer -- documentation
 *
 * @brief       Compares the version numbers of images A and B to determine
 *              which image is newer.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      If the version number of image B is larger than that of A,
 *              return true. Otherwise returns false.
 */
bool isImgBVersionNewer(void)
{
    uint8_t imgVerBuf[2];

    // Read image version information to determine most recent one
    memcpy(imgVerBuf, (uint32_t *)((BIM_IMG_B_PAGE * HAL_FLASH_PAGE_SIZE)
           + BIM_IMG_B_OSET + BIM_VER_OSET), 2);

    uint8_t imgBMajorVer = imgVerBuf[0];
    uint8_t imgBMinorVer = imgVerBuf[1];

    memcpy(imgVerBuf, (uint32_t *)((BIM_IMG_A_PAGE * HAL_FLASH_PAGE_SIZE)
           + BIM_IMG_A_OSET + BIM_VER_OSET), 2);

    uint8_t imgAMajorVer = imgVerBuf[0];
    uint8_t imgAMinorVer = imgVerBuf[1];

    // Compare image versions. If both images are the same version, then image B
    // takes precedence.
    if (imgBMajorVer > imgAMajorVer)
    {
        return true;
    }
    else if (imgBMajorVer < imgAMajorVer)
    {
        return false;
    }
    else
    {
        if (imgBMinorVer >= imgAMinorVer)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}


/*******************************************************************************
 * @fn          main
 *
 * @brief       C-code main function.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 */
void main(void)
{
    // Perform crc checks to verify image integrity
    bool validImgB = isCrcValid(BIM_IMG_B_PAGE, BIM_IMG_B_OSET);

#if !defined (FEATURE_FIXED_IMAGE)
    bool validImgA = isCrcValid(BIM_IMG_A_PAGE, BIM_IMG_A_OSET);
#else
    bool validImgA = true;
#endif // FEATURE_FIXED_IMAGE

    // Compare image versions to determine the most recent one if both are valid.
    if (validImgA && validImgB)
    {
        // If image A is newer than image B, then prevent image B from loading.
        validImgB = isImgBVersionNewer();
    }

    // Load images
    if (validImgB)
    {
        // Load address of the resetISR from the fixed location in Flash.
        asm(" MOV R0, #0x1001 ");

        // Because address 0x10010 is not a 16 bit value, the address must be
        // shifted to point to the correct location.
        asm(" LSL R0, #0x04");
        asm(" LDR R1, [R0, #0x4] ");

        // Reset the stack pointer,
        asm(" LDR SP, [R0, #0x0] ");

        // And jump.
        asm(" BX R1 ");
    }
    else if (validImgA)
    {
        // Load address of the resetISR from the fixed location in Flash.
        asm(" MOV R0, #0x1010 ");
        asm(" LDR R1, [R0, #0x4] ");

        // Reset the stack pointer,
        asm(" LDR SP, [R0, #0x0] ");

        // And jump.
        asm(" BX R1 ");
    }

}


/**************************************************************************************************
*/
