/******************************************************************************

 @file  oad_target_internal_flash.c

 @brief This file contains the externalFlash target implementation of the
        OAD profile.

 Group: WCS, BTS
 Target Device: CC1350

 ******************************************************************************
 
 Copyright (c) 2014-2017, Texas Instruments Incorporated
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
 Release Name: ti-ble-2.3.1-stack-sdk_1_40_xx
 Release Date: 2017-06-21 08:53:45
 *****************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include <string.h>

#include <ti/sysbios/hal/Hwi.h>
#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/vims.h)
#include DeviceFamily_constructPath(driverlib/flash.h)
#include DeviceFamily_constructPath(driverlib/sys_ctrl.h)

#include "oad_target.h"

/*********************************************************************
 * MACROS and CONSTANTS
 */

#define MAX_WRITE_INCREMENT   8

#define FLASH_ADDRESS(page, offset) (((page) << 12) + (offset))

#if !defined (OAD_IMAGE_A_USER_ID)
  #define OAD_IMAGE_A_USER_ID  {'A', 'A', 'A', 'A'}
#endif // OAD_IMAGE_A_USER_ID

#if !defined (OAD_IMAGE_B_USER_ID)
  #define OAD_IMAGE_B_USER_ID  {'B', 'B', 'B', 'B'}
#endif // OAD_IMAGE_B_USER_ID

#if !defined (OAD_IMAGE_VERSION)
  #define OAD_IMAGE_VERSION    0x0000
#endif // OAD_IMAGE_VERSION

#define HAL_FLASH_WORD_SIZE   4
#define HAL_FLASH_PAGE_SIZE   4096
/*********************************************************************
 * IMAGE HEADER
 */

// The CRC/CRC Shadow precedes the header and is not part of it.
// The header immediately follows the CRC at the beginning of the image.

// Not in use for cc13xx on-chip OAD
/*
#pragma location="IMAGE_HEADER"
const OADTarget_ImgHdr_t _imgHdr = {
  OAD_IMG_VER(OAD_IMAGE_VERSION), //15 bits, LSB identifies Image A/B build.
  // Number of flash words in image.
  OAD_IMG_R_AREA * OAD_FLASH_PAGE_MULT - (OAD_IMG_R_OSET / HAL_FLASH_WORD_SIZE),
#if defined HAL_IMAGE_A
  OAD_IMAGE_A_USER_ID,        // User-Id, Image A
#else // HAL_IMAGE_B
  OAD_IMAGE_B_USER_ID,        // User-Id, Image B
#endif // HAL_IMAGE_A
  { 0xFF, 0xFF, 0xFF, 0xFF }  // Reserved
};
#pragma required=_imgHdr
*/

/*********************************************************************
 * FUNCTIONS
 */

/*********************************************************************
 * @fn      OADTarget_open
 *
 * @brief   Open an OAD target for download. For internal flash
 *          this function is a stub.
 *
 * @param   none
 *
 * @return  Always TRUE for internal flash
 */
uint8_t OADTarget_open(void)
{
  return true;
}

/*********************************************************************
 * @fn      OADTarget_close
 *
 * @brief   Close an OAD target after a download has finished. For
 *          internal flash this function is a stub.
 *
 * @param   none
 *
 * @return  none
 */
void OADTarget_close(void)
{
  // Intentionally left blank.
}

/*********************************************************************
 * @fn      OADTarget_hasExternalFlash
 *
 * @brief   Check if the target has external flash
 *
 * @param   none
 *
 * @return  Always return FALSE, there is no external flash.
 */
uint8_t OADTarget_hasExternalFlash(void)
{
  return false;
}

/*********************************************************************
 * @fn      OADTarget_getCurrentImageHeader
 *
 * @brief   Get the current image's header.
 *
 * @param   pValue - pointer to the new image header
 *
 * @return  always TRUE
 */
uint8_t OADTarget_getCurrentImageHeader(OADTarget_ImgHdr_t *pHdr)
{
  memcpy((uint8_t *)pHdr,
         (uint8_t *)FLASH_ADDRESS(OAD_IMG_R_PAGE, OAD_IMG_R_OSET + OAD_IMG_HDR_OSET),
         sizeof(OADTarget_ImgHdr_t));

  return true;
}

/*********************************************************************
 * @fn      OADTarget_validateNewImage
 *
 * @brief   Determine if a new image should be downloaded or not based on
 *          target specific criteria.
 *
 * @param   pValue - pointer to new Image header information
 * @param   ImgHdr - pointer to contents of current image header
 * @param   blkTot - total number of blocks comprising new image.
 *
 * @return  TRUE to begin OAD otherwise FALSE to reject the image.
 */
uint8_t OADTarget_validateNewImage(uint8_t *pValue, OADTarget_ImgHdr_t *ImgHdr,
                                   uint16_t blkTot)
{
  uint8_t ret = false;
  OADTarget_ImgHdr_t rxHdr;

  rxHdr.ver = OADTarget_BUILD_UINT16(pValue[0], pValue[1]);
  rxHdr.len = OADTarget_BUILD_UINT16(pValue[2], pValue[3]);

  /* Requirements to begin OAD:
   * 1) LSB of image version cannot be the same, this would imply a code overlap
   *    between currently running image and new image.
   * 2) Total blocks of new image must not exceed maximum blocks supported, else
   *    the new image cannot fit.
   * 3) Block total must be greater than 0.
   * 4) Optional: Add additional criteria for initiating OAD here.
   */
  if ((OAD_IMG_ID(rxHdr.ver) == 0 || (OAD_IMG_ID(ImgHdr->ver) != OAD_IMG_ID(rxHdr.ver))) &&
      (blkTot <= OAD_BLOCK_MAX) &&
      (blkTot != 0))
  {
    ret = true;
  }

  return ret;
}

/*********************************************************************
 * @fn      OADTarget_storeImageHeader
 *
 * @brief   Store the image header of the new image
 *
 * @param   pValue - pointer to the new image header
 *
 * @return  none
 */
void OADTarget_storeImageHeader(uint8_t *pValue)
{
  // Do nothing.
}

/*********************************************************************
 * @fn      OADTarget_imageAddress
 *
 * @brief   Get the address of the current image
 *
 * @param   pValue - pointer to the new image header
 *
 * @param   pHdr - pointer to the current image header
 *
 * @return  address
 */
uint32_t OADTarget_imageAddress(uint8_t *pValue)
{
  // For internal flash, this is always the first byte of the download page.
  return OAD_IMG_D_PAGE << 12;
}

/*********************************************************************
 * @fn      OADTarget_getCrc
 *
 * @brief   Get the CRC array from the image that is being downloaded
 *
 * @param   pCrc - pointer to the new image header
 *
 * @return  None
 */
void OADTarget_getCrc(uint16_t *pCrc)
{
  // Read CRC and CRC shadow into buffer.
  OADTarget_readFlash(OAD_IMG_D_PAGE, OAD_IMG_CRC_OSET, (uint8_t *)pCrc,
                      sizeof(uint16_t) * 2);
}

/*********************************************************************
 * @fn      OADTarget_setCrc
 *
 * @brief   Set the CRC shadow of the downloaded image.
 *
 * @param   pCrc - pointer to the new image header
 *
 * @return  None.
 */
void OADTarget_setCrc(uint16_t *pCrc)
{
  // Write CRC shadow to downloaded image.
  OADTarget_writeFlash(OAD_IMG_D_PAGE, OAD_IMG_CRC_OSET, (uint8_t *)pCrc,
                       sizeof(uint16_t) * 2);

  // Read the CRC and CRC shadow back from downloaded image.
  OADTarget_readFlash(OAD_IMG_D_PAGE, OAD_IMG_CRC_OSET, (uint8_t *)pCrc,
                      sizeof(uint16_t) * 2);
}

/*********************************************************************
 * @fn      OADTarget_enableCache
 *
 * @brief   Prepares system for a write to flash, if necessary.
 *
 * @param   None.
 *
 * @return  None.
 */
static void OADTarget_enableCache(uint8_t state)
{
  if (state != VIMS_MODE_DISABLED)
  {
    // Enable the Cache.
    VIMSModeSet(VIMS_BASE, VIMS_MODE_ENABLED);
  }
}

/*********************************************************************
 * @fn      OADTarget_disableCache
 *
 * @brief   Resumes system after a write to flash, if necessary.
 *
 * @param   None.
 *
 * @return  VIMS_MODE_ENABLED if cache was in use before this operation,
 *          VIMS_MODE_DISABLED otherwise.
 */
static uint8_t OADTarget_disableCache(void)
{
  uint8_t state = VIMSModeGet(VIMS_BASE);

  // Check VIMS state
  if (state != VIMS_MODE_DISABLED)
  {
    // Invalidate cache
    VIMSModeSet(VIMS_BASE, VIMS_MODE_DISABLED);

    // Wait for disabling to be complete
    while (VIMSModeGet(VIMS_BASE) != VIMS_MODE_DISABLED);
  }

  return state;
}

/*********************************************************************
 * @fn      OADTarget_readFlash
 *
 * @brief   Read data from flash.
 *
 * @param   page   - page to read from in flash
 * @param   offset - offset into flash page to begin reading
 * @param   pBuf   - pointer to buffer into which data is read.
 * @param   len    - length of data to read in bytes.
 *
 * @return  None.
 */
void OADTarget_readFlash(uint8_t page, uint32_t offset, uint8_t *pBuf,
                         uint16_t len)
{

  uint8_t *ptr = (uint8_t *)FLASH_ADDRESS(page, offset);

  // Enter critical section.
  uint32_t key = Hwi_disable();

  // Read from pointer into buffer.
  while (len--)
  {
    *pBuf++ = *ptr++;
  }

  // Exit critical section.
  Hwi_restore(key);
}

/*********************************************************************
 * @fn      OADTarget_writeFlash
 *
 * @brief   Write data to flash.
 *
 * @param   page   - page to write to in flash
 * @param   offset - offset into flash page to begin writing
 * @param   pBuf   - pointer to buffer of data to write
 * @param   len    - length of data to write in bytes
 *
 * @return  None.
 */
void OADTarget_writeFlash(uint8_t page, uint32_t offset, uint8_t *pBuf,
                          uint16_t len)
{
  unsigned int key;
  uint8_t cacheState;
  uint8_t writeIncrement;
  uint8_t status;

  cacheState = OADTarget_disableCache();

  while (len)
  {
    if (len > MAX_WRITE_INCREMENT)
    {
      writeIncrement = MAX_WRITE_INCREMENT; 
    }
    else 
    { 
      writeIncrement = len;
    }
    key = Hwi_disable();
    status = FlashProgram(pBuf, (uint32_t)FLASH_ADDRESS(page, offset), writeIncrement);
    Hwi_restore(key);

    if (status != 0)
    {
      while(1); //Error writing to flash 
    }
    else
    { 
      len -= writeIncrement;
      pBuf += writeIncrement; 
      offset += writeIncrement; 
    }
  }

  OADTarget_enableCache(cacheState);
}

/*********************************************************************
 * @fn      OADTarget_eraseFlash
 *
 * @brief   Erase selected flash page.
 *
 * @param   page - the page to erase.
 *
 * @return  None.
 */
void OADTarget_eraseFlash(uint8_t page)
{
  uint8_t cacheState;
  unsigned int key; 

  cacheState = OADTarget_disableCache();

  key = Hwi_disable();
  FlashSectorErase((uint32_t)FLASH_ADDRESS(page, 0));
  Hwi_restore(key);

  OADTarget_enableCache(cacheState);
}

/*********************************************************************
 * @fn      OADTarget_systemReset
 *
 * @brief   Prepare system for a reset and trigger a reset to the boot
 *          image manager.
 *
 * @param   None.
 *
 * @return  None.
 */
void OADTarget_systemReset(void)
{
  SysCtrlSystemReset();
}

/*******************************************************************************
 * @fn      saveImageInfo
 *
 * @brief   Save image information in the meta-data area
 *
 * @return  none
 */
void saveImageInfo(void)
{
  // Do nothing.
}

/*******************************************************************************
 * @fn      getImageFlag
 *
 * @brief   Not used
 *
 * @param   None.
 *
 * @return  0
 */
extern uint8_t getImageFlag(void)
{
  return 0;
}
