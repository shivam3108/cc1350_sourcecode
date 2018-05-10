/******************************************************************************

 @file oad_storage.c

 @brief OAD Storage

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

/*********************************************************************
 * INCLUDES
 */
#include <string.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/flash.h)

#include "oad/native_oad/oad_protocol.h"
#include "oad/native_oad/oad_storage.h"

/*********************************************************************
 * CONSTANTS
 */
#define ERROR_BLOCK     0xFFFF

#define HAL_FLASH_WORD_SIZE  4

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

static uint16_t oadBlkTot = 0xFFFF;
static uint16_t oadRemianingBytes = 0;
static uint32_t imageAddress;
static uint16_t imagePage;
static uint32_t flashPageSize;

#ifndef FEATURE_OAD_ONCHIP
// Used to keep track of images written.
static uint8_t flagRecord = 0;
#endif //FEATURE_OAD_ONCHIP

static uint8_t OADStorage_imageIdLen = 0;

// Flash consists of 32 pages of 4 KB.
#define HAL_FLASH_WORD_SIZE       4

/*********************************************************************
 * LOCAL FUNCTIONS
 */

#if !defined FEATURE_OAD_ONCHIP
static uint8_t checkDL(void);
static uint16_t crcCalcDL(void);
static uint16_t crc16(uint16_t crc, uint8_t val);
#endif  // !FEATURE_OAD_ONCHIP


/*********************************************************************
 * @fn      OADStorage_register
 *
 * @brief   Register a callback function with the OAD Target Profile.
 *
 * @param   *pfnOadCBs - write callback function container.
 *
 * @return  None.
 */
void OADStorage_init(void)
{

  // Set flash size variables
  flashPageSize = FlashSectorSizeGet();
}

/*********************************************************************
 * @fn      OADStorage_imgIdentifyRead
 *
* @brief   Read Image header and return number of blocks.
 *
 * @param   imageType   - image type indicating which image to read
 * @param   pImgHdr     - pointer to image header data
 *
 * @return  Total Blocks if image accepted, 0 if Image invalid
 */
uint16_t OADStorage_imgIdentifyRead(uint8_t imageType, OADTarget_ImgHdr_t* pImgHdr)
{
    uint16_t oadBlkTot;
    uint16_t remianingBytes;

    OADTarget_getImageHeader(imageType, pImgHdr);

    if(pImgHdr->len != 0x0000)
    {
        OADTarget_open();

        // Determine where image will be read from.
        imageAddress = OADTarget_imageAddress((uint8_t*)pImgHdr);
        imagePage = imageAddress / FlashSectorSizeGet();

        // Calculate block total of the new image.
        oadBlkTot = pImgHdr->len /
                    (OAD_BLOCK_SIZE / 4);

        // Calculate remaining bytes
        remianingBytes = (pImgHdr->len * 4) - (oadBlkTot * OAD_BLOCK_SIZE);

        // Check for remaining in last block
        if(remianingBytes != 0)
        {
            oadBlkTot += 1;
        }

        return (int32_t) oadBlkTot;
    }
    else
    {
        return 0;
    }
}

/*********************************************************************
 * @fn      OADStorage_imgIdentifyWrite
 *
 * @brief   Process the Image Identify Write.  Determine from the received OAD
 *          Image Header if the Downloaded Image should be acquired.
 *
 * @param   pValue     - pointer to image header data
 *
 * @return  Total Blocks if image accepted, 0 if Image rejected
 */
uint16_t OADStorage_imgIdentifyWrite(uint8_t *pValue)
{
    OADTarget_ImgHdr_t ImgHdr;
    uint8_t hdrOffset = OADStorage_imageIdLen == 8 ? 0 : 4;

    // Set flash size variables
    flashPageSize = FlashSectorSizeGet();

    // Store the new image's header
    OADTarget_storeImageHeader(pValue);

    // Read out running image's header.
    OADTarget_getCurrentImageHeader(&ImgHdr);

    // Calculate block total of the new image
    oadBlkTot = OADTarget_BUILD_UINT16(pValue[hdrOffset + 2], pValue[hdrOffset + 3]) /
              (OAD_BLOCK_SIZE / HAL_FLASH_WORD_SIZE);

    // Calculate remaining bytes
    oadRemianingBytes = ((OADTarget_BUILD_UINT16(pValue[hdrOffset + 2], pValue[hdrOffset + 3])) * HAL_FLASH_WORD_SIZE) - (oadBlkTot * OAD_BLOCK_SIZE);

    // Check for remaining in last block
    if(oadRemianingBytes != 0)
    {
        oadBlkTot += 1;
    }

#ifndef FEATURE_OAD_ONCHIP
    flagRecord = 0;
#endif

    /* Requirements to begin OAD:
     * 1) LSB of image version cannot be the same, this would imply a code overlap
     *    between currently running image and new image.
     * 2) Total blocks of new image must not exceed maximum blocks supported, else
     *    the new image cannot fit.
     * 3) Block total must be greater than 0.
     * 4) Optional: Add additional criteria for initiating OAD here.
     */
    if (OADTarget_validateNewImage(pValue + hdrOffset, &ImgHdr, oadBlkTot))
    {
        // Determine where image will be stored.
        imageAddress = OADTarget_imageAddress(pValue+hdrOffset);
        imagePage = imageAddress / FlashSectorSizeGet();

        // Open the target interface
        if(OADTarget_open() == 0)
        {
            oadBlkTot = 0;
        }
    }
    else
    {
        oadBlkTot = 0;
    }

    return oadBlkTot;
}

/*********************************************************************
 * @fn      OADStorage_imgBlockWrite
 *
 * @brief   Write an Image Block.
 *
 * @param   blockNum   - block number to be written
 * @param   pBlockData - pointer to data to be written
 *
 * @return  none
 */
void OADStorage_imgBlockWrite(uint16_t blockNum, uint8_t *pBlockData)
{
    // Calculate address to write as (start of OAD range) + (offset into range)
    uint32_t addr = imageAddress + (blockNum * OAD_BLOCK_SIZE);

    // If address starts a new page, erase that page first.
    if ((addr % flashPageSize) == 0)
    {
      OADTarget_eraseFlash(addr / flashPageSize);
    }

    // Write a 16 byte block to Flash.
    OADTarget_writeFlash(imagePage, (blockNum * OAD_BLOCK_SIZE), pBlockData,
                         OAD_BLOCK_SIZE);
}

/*********************************************************************
 * @fn      OADStorage_imgBlockRead
 *
 * @brief   Read an Image block.
 *
 * @param   blockNum   - block number to be read
 * @param   pBlockData - pointer for data to be read
 *
 * @return  none
 */
void OADStorage_imgBlockRead(uint16_t blockNum, uint8_t *pBlockData)
{
    // Read a block from Flash.
    OADTarget_readFlash(imagePage, (blockNum * OAD_BLOCK_SIZE), pBlockData,
                         OAD_BLOCK_SIZE);
}

/*********************************************************************
 * @fn      OADStorage_imgInfoRead
 *
 * @brief   Read an Image info.
 *
 * @param   pimgInfo - pointer for data to be read
 *
 * @return  none
 */
void OADStorage_imgInfoRead(uint8_t *pimgInfo)
{
    // Read a block from Flash.
    OADTarget_readFlash(imagePage, 0, pimgInfo,
                         16);
}

/*********************************************************************
 * @fn      OADStorage_imgFinalise
 *
 * @brief   Process the Image Block Write.
 *
 * @param  none
 *
 * @return  OADStorage_Status_t
 */
OADStorage_Status_t OADStorage_imgFinalise(void)
{
    OADStorage_Status_t status = OADStorage_Status_Success;
    
// For onchip Handle CRC verification in BIM.
#if !defined FEATURE_OAD_ONCHIP
    // Run CRC check on new image.
    if (checkDL())
    {
        // Store the flag of the downloaded image.
        flagRecord |= getImageFlag();

        // Store the image information.
        saveImageInfo();

        status = OADStorage_Status_Success;
    }
    else
    {
        // CRC error
        status = OADStorage_CrcError;
    }
    flagRecord = 0;
#endif //!FEATURE_OAD_ONCHIP
    OADTarget_close();

    return status;
}

/*********************************************************************
 * @fn      OADStorage_imgFinalise
 *
 * @brief   Process the Image Block Write.
 *
 * @param  none
 *
 * @return none
 */
void OADStorage_close(void)
{
    OADTarget_close();
}

#if !defined FEATURE_OAD_ONCHIP
/*********************************************************************
 * @fn      crcCalcDL
 *
 * @brief   Run the CRC16 Polynomial calculation over the DL image.
 *
 * @param   None
 *
 * @return  The CRC16 calculated.
 */
static uint16_t crcCalcDL(void)
{
  uint16_t imageCRC = 0;
  uint8_t page;
  uint32_t flashPageSize = FlashSectorSizeGet();
  uint32_t blocksPerPage = flashPageSize / OAD_BLOCK_SIZE;

  uint8_t lastPage = oadBlkTot / blocksPerPage;

  // Calculate remaining blocks in last page.
  uint16_t numRemBytes = (oadBlkTot - (lastPage * blocksPerPage))
                         * OAD_BLOCK_SIZE;

  // Is the last block partially filled
  if(oadRemianingBytes != 0)
  {
      numRemBytes -= (OAD_BLOCK_SIZE - oadRemianingBytes);
  }

  // Set last page to end of OAD image address range.
  lastPage += imagePage;

  // Read over downloaded pages
  for (page = imagePage; page <= lastPage; page++)
  {
    uint16_t offset;

    // Read over all flash words in a page, excluding the CRC section of the
    // first page and all bytes after remainder bytes on the last page.
    for (offset = (page == imagePage) ? HAL_FLASH_WORD_SIZE : 0;
         offset < flashPageSize &&
         (page < lastPage || offset < numRemBytes);
         offset += HAL_FLASH_WORD_SIZE)
    {
      uint8_t buf[HAL_FLASH_WORD_SIZE];
      uint8_t idx;

      // Read a word from flash.
      OADTarget_readFlash(page, offset, buf, HAL_FLASH_WORD_SIZE);

      // Calculate CRC of word, byte by byte.
      for (idx = 0; idx < HAL_FLASH_WORD_SIZE; idx++)
      {
        imageCRC = crc16(imageCRC, buf[idx]);
      }
    }
  }

  // IAR note explains that poly must be run with value zero for each byte of
  // the crc.
  imageCRC = crc16(imageCRC, 0);
  imageCRC = crc16(imageCRC, 0);

  // Return the CRC calculated over the image.
  return imageCRC;
}

/*********************************************************************
 * @fn      checkDL
 *
 * @brief   Check validity of the downloaded image.
 *
 * @param   None.
 *
 * @return  TRUE or false for image valid.
 */
static uint8_t checkDL(void)
{
  uint16_t crc[2];

  OADTarget_getCrc(crc);

  if ((crc[0] == 0xFFFF) || (crc[0] == 0x0000))
  {
    return false;
  }

  // Calculate CRC of downloaded image.
  crc[1] = crcCalcDL();

  if (crc[1] == crc[0])
  {
    // Set the CRC shadow as equivalent to the CRC.
    OADTarget_setCrc(crc);
  }

  return (crc[0] == crc[1]);
}

/*********************************************************************
 * @fn          crc16
 *
 * @brief       Run the CRC16 Polynomial calculation over the byte parameter.
 *
 * @param       crc - Running CRC calculated so far.
 * @param       val - Value on which to run the CRC16.
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

#endif // !FEATURE_OAD_ONCHIP

/*********************************************************************
*********************************************************************/
