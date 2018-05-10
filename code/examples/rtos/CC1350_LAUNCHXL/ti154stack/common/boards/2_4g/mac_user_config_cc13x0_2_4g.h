/******************************************************************************

 @file mac_user_config_cc13xx_2_4g.h

 @brief override and power table

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
 
#ifndef _CONFIG_CC13X0_2_4G_H
#define _CONFIG_CC13X0_2_4G_H

/******************************************************************************
 Includes
 *****************************************************************************/
#include "mac_user_config_cc13x0_rftable.h"

/******************************************************************************
 Constants and definitions
 *****************************************************************************/

/******************************************************************************
 External Variables
 *****************************************************************************/

/******************************************************************************
 Global Variables
 *****************************************************************************/
//
// Tx Power Table Used Depends on Device Package
//
// RF patch pointers
//
const RF_Mode rfPatchTable_ieee =
{
     .rfMode = RF_MODE_IEEE_15_4,
     .cpePatchFxn = 0,
     .mcePatchFxn = 0,
     .rfePatchFxn = 0,
};

// Tx Power Table
const txPwrTbl_t txPwrTbl_ieee[] =
    { { txPowerTable_ieee } };  /* power table for CC1350 */

// RF Override Table
// PG2.1 and PG2.0 uses the same override table
const uint32_t *pOverridesTable_ieee[] =
    { (uint32_t *) pOverrides_ieee };    /* overrides for PG2.1 */

const uint8_t loDivider_ieee = MAC_LO_DIVIDER_5;

#endif
