/******************************************************************************

 @file mac_user_config_cc13x0_subg_cop.h

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

#ifndef _CONFIG_CC13X0_RF_TABLE_H
#define _CONFIG_CC13X0_RF_TABLE_H

/******************************************************************************
 Includes
 *****************************************************************************/

/******************************************************************************
 Constants and definitions
 *****************************************************************************/

/******************************************************************************
 External Variables
 *****************************************************************************/

/******************************************************************************
 Global Variables
 *****************************************************************************/
/* US BAND and ETSI BAND */
// Overrides for CMD_PROP_RADIO_DIV_SETUP
static uint32_t pOverrides_fsk_CC1350[] =
{
    // override_use_patch_prop_genfsk.xml
    // PHY: Use MCE ROM bank 4, RFE RAM patch
    MCE_RFE_OVERRIDE(0,4,0,1,0,0),
    // override_synth_prop_863_930_div5.xml
    // Synth: Set recommended RTRIM to 7
    HW_REG_OVERRIDE(0x4038,0x0037),
    // Synth: Set Fref to 4 MHz
    (uint32_t)0x000684A3,
    // Synth: Configure fine calibration setting
    HW_REG_OVERRIDE(0x4020,0x7F00),
    // Synth: Configure fine calibration setting
    HW_REG_OVERRIDE(0x4064,0x0040),
    // Synth: Configure fine calibration setting
    (uint32_t)0xB1070503,
    // Synth: Configure fine calibration setting
    (uint32_t)0x05330523,
    // Synth: Set loop bandwidth after lock to 20 kHz
    (uint32_t)0x0A480583,
    // Synth: Set loop bandwidth after lock to 20 kHz
    (uint32_t)0x7AB80603,
    // Synth: Configure VCO LDO (in ADI1, set VCOLDOCFG=0x9F to use voltage input reference)
    ADI_REG_OVERRIDE(1,4,0x9F),
    // Synth: Configure synth LDO (in ADI1, set SLDOCTL0.COMP_CAP=1)
    ADI_HALFREG_OVERRIDE(1,7,0x4,0x4),
    // Synth: Use 24 MHz XOSC as synth clock, enable extra PLL filtering
    (uint32_t)0x02010403,
    // Synth: Configure extra PLL filtering
    (uint32_t)0x00108463,
    // Synth: Increase synth programming timeout (0x04B0 RAT ticks = 300 us)
    (uint32_t)0x04B00243,
    // override_phy_rx_aaf_bw_0xd.xml
    // Rx: Set anti-aliasing filter bandwidth to 0xD (in ADI0, set IFAMPCTL3[7:4]=0xD)
    ADI_HALFREG_OVERRIDE(0,61,0xF,0xD),
    // override_phy_gfsk_rx.xml
    // Rx: Set LNA bias current trim offset to 3
    (uint32_t)0x00038883,
    // Rx: Freeze RSSI on sync found event
    HW_REG_OVERRIDE(0x6084,0x35F1),
    // override_phy_gfsk_pa_ramp_agc_reflevel_0x1a.xml
    // Tx: Configure PA ramping setting (0x41). Rx: Set AGC reference level to 0x1A.
    HW_REG_OVERRIDE(0x6088,0x411A),
    // Tx: Configure PA ramping setting
    HW_REG_OVERRIDE(0x608C,0x8213),
    // override_crc_ieee_802_15_4.xml
    // IEEE 802.15.4g: Fix incorrect initialization value for CRC-16 calculation (see TRM section 23.7.5.2.1)
    (uint32_t)0x00000943,
    // IEEE 802.15.4g: Fix incorrect initialization value for CRC-16 calculation (see TRM section 23.7.5.2.1)
    (uint32_t)0x00000963,
    // override_phy_rx_rssi_offset_5db.xml
    // Rx: Set RSSI offset to adjust reported RSSI by +5 dB
    (uint32_t)0x00FB88A3,
#if CCFG_FORCE_VDDR_HH
    // Tx: Set PA trim to max (in ADI0, set PACTL0=0xF8)
    ADI_REG_OVERRIDE(0,12,0xF8),
#endif
    (uint32_t)0x00000943, // Correct CRC initialization for 16-bit CRC
    (uint32_t)0x00000963, // Correct CRC initialization for 16-bit CRC
    (uint32_t)0xFFFFFFFF,
};

static uint32_t pOverrides_slr_CC1350[] =
{
    // override_use_patch_simplelink_long_range.xml
    // PHY: Use MCE RAM patch, RFE RAM patch
    MCE_RFE_OVERRIDE(1,0,0,1,0,0),
    // override_synth_prop_863_930_div5_lbw60k.xml
    // Synth: Set recommended RTRIM to 7
    HW_REG_OVERRIDE(0x4038,0x0037),
    // Synth: Set Fref to 4 MHz
    (uint32_t)0x000684A3,
    // Synth: Configure fine calibration setting
    HW_REG_OVERRIDE(0x4020,0x7F00),
    // Synth: Configure fine calibration setting
    HW_REG_OVERRIDE(0x4064,0x0040),
    // Synth: Configure fine calibration setting
    (uint32_t)0xB1070503,
    // Synth: Configure fine calibration setting
    (uint32_t)0x05330523,
    // Synth: Set loop bandwidth after lock to 60 kHz
    (uint32_t)0x40410583,
    // Synth: Set loop bandwidth after lock to 60 kHz
    (uint32_t)0x32CC0603,
    // Synth: Set loop bandwidth after lock to 60 kHz
    (uint32_t)0x00010623,
    // Synth: Configure VCO LDO (in ADI1, set VCOLDOCFG=0x9F to use voltage input reference)
    ADI_REG_OVERRIDE(1,4,0x9F),
    // Synth: Configure synth LDO (in ADI1, set SLDOCTL0.COMP_CAP=1)
    ADI_HALFREG_OVERRIDE(1,7,0x4,0x4),
    // Synth: Use 24 MHz XOSC as synth clock, enable extra PLL filtering
    (uint32_t)0x02010403,
    // Synth: Configure extra PLL filtering
    (uint32_t)0x00108463,
    // Synth: Increase synth programming timeout (0x04B0 RAT ticks = 300 us)
    (uint32_t)0x04B00243,
    // override_synth_disable_bias_div5.xml
    // Synth: Set divider bias to disabled
    HW32_ARRAY_OVERRIDE(0x405C,1),
    // Synth: Set divider bias to disabled (specific for loDivider=5)
    (uint32_t)0x18000200,
    // override_phy_rx_aaf_bw_0xd.xml
    // Rx: Set anti-aliasing filter bandwidth to 0xD (in ADI0, set IFAMPCTL3[7:4]=0xD)
    ADI_HALFREG_OVERRIDE(0,61,0xF,0xD),
    // override_phy_gfsk_rx.xml
    // Rx: Set LNA bias current trim offset to 3
    (uint32_t)0x00038883,
    // Rx: Freeze RSSI on sync found event
    HW_REG_OVERRIDE(0x6084,0x35F1),
    // override_phy_gfsk_pa_ramp_agc_reflevel_0x14.xml
    // Tx: Configure PA ramping setting (0x41). Rx: Set AGC reference level to 0x14.
    HW_REG_OVERRIDE(0x6088,0x4114),
    // Tx: Configure PA ramping setting
    HW_REG_OVERRIDE(0x608C,0x8213),
    // override_phy_long_range_dsss2.xml
    // PHY: Configure DSSS SF=2
    HW_REG_OVERRIDE(0x505C,0x0100),
    // override_phy_rx_rssi_offset_5db.xml
    // Rx: Set RSSI offset to adjust reported RSSI by +5 dB
    (uint32_t)0x00FB88A3,
    // TX power override
#if CCFG_FORCE_VDDR_HH
    // Tx: Set PA trim to max (in ADI0, set PACTL0=0xF8)
    ADI_REG_OVERRIDE(0,12,0xF8),
#endif
    (uint32_t)0x00000943, // Correct CRC initialization for 16-bit CRC
    (uint32_t)0x00000963, // Correct CRC initialization for 16-bit CRC
    (uint32_t)0xFFFFFFFF,
};

static uint32_t pOverrides_fsk_CC1310_CC1190[] =
{
    // override_use_patch_prop_genfsk.xml
    // PHY: Use MCE ROM bank 4, RFE RAM patch
    MCE_RFE_OVERRIDE(0,4,0,1,0,0),
    // override_synth_prop_863_930_div5.xml
    // Synth: Set recommended RTRIM to 7
    HW_REG_OVERRIDE(0x4038,0x0037),
    // Synth: Set Fref to 4 MHz
    (uint32_t)0x000684A3,
    // Synth: Configure fine calibration setting
    HW_REG_OVERRIDE(0x4020,0x7F00),
    // Synth: Configure fine calibration setting
    HW_REG_OVERRIDE(0x4064,0x0040),
    // Synth: Configure fine calibration setting
    (uint32_t)0xB1070503,
    // Synth: Configure fine calibration setting
    (uint32_t)0x05330523,
    // Synth: Set loop bandwidth after lock to 20 kHz
    (uint32_t)0x0A480583,
    // Synth: Set loop bandwidth after lock to 20 kHz
    (uint32_t)0x7AB80603,
    // Synth: Configure VCO LDO (in ADI1, set VCOLDOCFG=0x9F to use voltage input reference)
    ADI_REG_OVERRIDE(1,4,0x9F),
    // Synth: Configure synth LDO (in ADI1, set SLDOCTL0.COMP_CAP=1)
    ADI_HALFREG_OVERRIDE(1,7,0x4,0x4),
    // Synth: Use 24 MHz XOSC as synth clock, enable extra PLL filtering
    (uint32_t)0x02010403,
    // Synth: Configure extra PLL filtering
    (uint32_t)0x00108463,
    // Synth: Increase synth programming timeout (0x04B0 RAT ticks = 300 us)
    (uint32_t)0x04B00243,
    // override_phy_rx_aaf_bw_0xd.xml
    // Rx: Set anti-aliasing filter bandwidth to 0xD (in ADI0, set IFAMPCTL3[7:4]=0xD)
    ADI_HALFREG_OVERRIDE(0,61,0xF,0xD),
    // override_phy_gfsk_rx.xml
    // Rx: Set LNA bias current trim offset to 3
    (uint32_t)0x00038883,
    // Rx: Freeze RSSI on sync found event
    HW_REG_OVERRIDE(0x6084,0x35F1),
    // override_phy_gfsk_pa_ramp_agc_reflevel_0x1a.xml
    // Tx: Configure PA ramping setting (0x41). Rx: Set AGC reference level to 0x1A.
    HW_REG_OVERRIDE(0x6088,0x411A),
    // Tx: Configure PA ramping setting
    HW_REG_OVERRIDE(0x608C,0x8213),
    // override_crc_ieee_802_15_4.xml
    // IEEE 802.15.4g: Fix incorrect initialization value for CRC-16 calculation (see TRM section 23.7.5.2.1)
    (uint32_t)0x00000943,
    // IEEE 802.15.4g: Fix incorrect initialization value for CRC-16 calculation (see TRM section 23.7.5.2.1)
    (uint32_t)0x00000963,
    // override_phy_rx_rssi_offset_cc1310_cc1190_869.xml
    // Rx: Set RSSI offset to adjust reported RSSI by +26 dB
    (uint32_t)0x000188A3,
    // TX power override
    // Tx: Set PA trim to max (in ADI0, set PACTL0=0xF8)
    ADI_REG_OVERRIDE(0,12,0xF8),
    (uint32_t)0x00000943, // Correct CRC initialization for 16-bit CRC
    (uint32_t)0x00000963, // Correct CRC initialization for 16-bit CRC
    (uint32_t)0xFFFFFFFF,
};

static uint32_t pOverrides_slr_CC1310_CC1190[] =
{
    // override_use_patch_simplelink_long_range.xml
    // PHY: Use MCE RAM patch, RFE RAM patch
    MCE_RFE_OVERRIDE(1,0,0,1,0,0),
    // override_synth_prop_863_930_div5_lbw60k.xml
    // Synth: Set recommended RTRIM to 7
    HW_REG_OVERRIDE(0x4038,0x0037),
    // Synth: Set Fref to 4 MHz
    (uint32_t)0x000684A3,
    // Synth: Configure fine calibration setting
    HW_REG_OVERRIDE(0x4020,0x7F00),
    // Synth: Configure fine calibration setting
    HW_REG_OVERRIDE(0x4064,0x0040),
    // Synth: Configure fine calibration setting
    (uint32_t)0xB1070503,
    // Synth: Configure fine calibration setting
    (uint32_t)0x05330523,
    // Synth: Set loop bandwidth after lock to 60 kHz
    (uint32_t)0x40410583,
    // Synth: Set loop bandwidth after lock to 60 kHz
    (uint32_t)0x32CC0603,
    // Synth: Set loop bandwidth after lock to 60 kHz
    (uint32_t)0x00010623,
    // Synth: Configure VCO LDO (in ADI1, set VCOLDOCFG=0x9F to use voltage input reference)
    ADI_REG_OVERRIDE(1,4,0x9F),
    // Synth: Configure synth LDO (in ADI1, set SLDOCTL0.COMP_CAP=1)
    ADI_HALFREG_OVERRIDE(1,7,0x4,0x4),
    // Synth: Use 24 MHz XOSC as synth clock, enable extra PLL filtering
    (uint32_t)0x02010403,
    // Synth: Configure extra PLL filtering
    (uint32_t)0x00108463,
    // Synth: Increase synth programming timeout (0x04B0 RAT ticks = 300 us)
    (uint32_t)0x04B00243,
    // override_synth_disable_bias_div5.xml
    // Synth: Set divider bias to disabled
    HW32_ARRAY_OVERRIDE(0x405C,1),
    // Synth: Set divider bias to disabled (specific for loDivider=5)
    (uint32_t)0x18000200,
    // override_phy_rx_aaf_bw_0xd.xml
    // Rx: Set anti-aliasing filter bandwidth to 0xD (in ADI0, set IFAMPCTL3[7:4]=0xD)
    ADI_HALFREG_OVERRIDE(0,61,0xF,0xD),
    // override_phy_gfsk_rx.xml
    // Rx: Set LNA bias current trim offset to 3
    (uint32_t)0x00038883,
    // Rx: Freeze RSSI on sync found event
    HW_REG_OVERRIDE(0x6084,0x35F1),
    // override_phy_gfsk_pa_ramp_agc_reflevel_0x14.xml
    // Tx: Configure PA ramping setting (0x41). Rx: Set AGC reference level to 0x14.
    HW_REG_OVERRIDE(0x6088,0x4114),
    // Tx: Configure PA ramping setting
    HW_REG_OVERRIDE(0x608C,0x8213),
    // override_phy_long_range_dsss2.xml
    // PHY: Configure DSSS SF=2
    HW_REG_OVERRIDE(0x505C,0x0100),
    // override_phy_rx_rssi_offset_cc1310_cc1190_869.xml
    // Rx: Set RSSI offset to adjust reported RSSI by +26 dB
    (uint32_t)0x000188A3,
    // TX power override
    // Tx: Set PA trim to max (in ADI0, set PACTL0=0xF8)
    ADI_REG_OVERRIDE(0,12,0xF8),
    (uint32_t)0x00000943, // Correct CRC initialization for 16-bit CRC
    (uint32_t)0x00000963, // Correct CRC initialization for 16-bit CRC
    (uint32_t)0xFFFFFFFF,
};

/* CHINA BAND */
static uint32_t pOverrides_fsk_433_CC1350[] =
{
    // override_use_patch_prop_genfsk.xml
    // PHY: Use MCE ROM bank 4, RFE RAM patch
    MCE_RFE_OVERRIDE(0,4,0,1,0,0),
    // override_synth_prop_430_510_div10.xml
    // Synth: Set recommended RTRIM to 7
    HW_REG_OVERRIDE(0x4038,0x0037),
    // Synth: Set Fref to 4 MHz
    (uint32_t)0x000684A3,
    // Synth: Configure fine calibration setting
    HW_REG_OVERRIDE(0x4020,0x7F00),
    // Synth: Configure fine calibration setting
    HW_REG_OVERRIDE(0x4064,0x0040),
    // Synth: Configure fine calibration setting
    (uint32_t)0xB1070503,
    // Synth: Configure fine calibration setting
    (uint32_t)0x05330523,
    // Synth: Set loop bandwidth after lock to 20 kHz
    (uint32_t)0x0A480583,
    // Synth: Set loop bandwidth after lock to 20 kHz
    (uint32_t)0x7AB80603,
    // Synth: Configure VCO LDO (in ADI1, set VCOLDOCFG=0x9F to use voltage input reference)
    ADI_REG_OVERRIDE(1,4,0x9F),
    // Synth: Configure synth LDO (in ADI1, set SLDOCTL0.COMP_CAP=1)
    ADI_HALFREG_OVERRIDE(1,7,0x4,0x4),
    // Synth: Use 24 MHz XOSC as synth clock, enable extra PLL filtering
    (uint32_t)0x02010403,
    // Synth: Configure extra PLL filtering
    (uint32_t)0x00108463,
    // Synth: Increase synth programming timeout (0x04B0 RAT ticks = 300 us)
    (uint32_t)0x04B00243,
    // override_synth_disable_bias_div10.xml
    // Synth: Set divider bias to disabled
    HW32_ARRAY_OVERRIDE(0x405C,1),
    // Synth: Set divider bias to disabled (specific for loDivider=10)
    (uint32_t)0x18000280,
    // override_phy_rx_aaf_bw_0xd.xml
    // Rx: Set anti-aliasing filter bandwidth to 0xD (in ADI0, set IFAMPCTL3[7:4]=0xD)
    ADI_HALFREG_OVERRIDE(0,61,0xF,0xD),
    // override_phy_gfsk_rx.xml
    // Rx: Set LNA bias current trim offset to 3
    (uint32_t)0x00038883,
    // Rx: Freeze RSSI on sync found event
    HW_REG_OVERRIDE(0x6084,0x35F1),
    // override_phy_gfsk_pa_ramp_agc_reflevel_0x1a.xml
    // Tx: Configure PA ramping setting (0x41). Rx: Set AGC reference level to 0x1A.
    HW_REG_OVERRIDE(0x6088,0x411A),
    // Tx: Configure PA ramping setting
    HW_REG_OVERRIDE(0x608C,0x8213),
    // override_phy_rx_rssi_offset_neg2db.xml
    // Rx: Set RSSI offset to adjust reported RSSI by -2 dB
    (uint32_t)0x000288A3,
    // TX power override
#if CCFG_FORCE_VDDR_HH
    // Tx: Set PA trim to max (in ADI0, set PACTL0=0xF8)
    ADI_REG_OVERRIDE(0,12,0xF8),
#endif
    (uint32_t)0x00000943, // Correct CRC initialization for 16-bit CRC
    (uint32_t)0x00000963, // Correct CRC initialization for 16-bit CRC
    (uint32_t)0xFFFFFFFF,
};

/* ToDo : this override table need to be updated : probably not working */
static uint32_t pOverrides_slr_433_CC1350[] =
{
    // override_use_patch_simplelink_long_range.xml
    // PHY: Use MCE RAM patch, RFE RAM patch
    MCE_RFE_OVERRIDE(1,0,0,1,0,0),
    // override_synth_prop_430_510_div10_lbw60k.xml
    // Synth: Set recommended RTRIM to 7
    HW_REG_OVERRIDE(0x4038,0x0037),
    // Synth: Set Fref to 4 MHz
    (uint32_t)0x000684A3,
    // Synth: Configure fine calibration setting
    HW_REG_OVERRIDE(0x4020,0x7F00),
    // Synth: Configure fine calibration setting
    HW_REG_OVERRIDE(0x4064,0x0040),
    // Synth: Configure fine calibration setting
    (uint32_t)0xB1070503,
    // Synth: Configure fine calibration setting
    (uint32_t)0x05330523,
    // Synth: Set loop bandwidth after lock to 60 kHz
    (uint32_t)0x40410583,
    // Synth: Set loop bandwidth after lock to 60 kHz
    (uint32_t)0x32CC0603,
    // Synth: Set loop bandwidth after lock to 60 kHz
    (uint32_t)0x00010623,
    // Synth: Configure VCO LDO (in ADI1, set VCOLDOCFG=0x9F to use voltage input reference)
    ADI_REG_OVERRIDE(1,4,0x9F),
    // Synth: Configure synth LDO (in ADI1, set SLDOCTL0.COMP_CAP=1)
    ADI_HALFREG_OVERRIDE(1,7,0x4,0x4),
    // Synth: Use 24 MHz XOSC as synth clock, enable extra PLL filtering
    (uint32_t)0x02010403,
    // Synth: Configure extra PLL filtering
    (uint32_t)0x00108463,
    // Synth: Increase synth programming timeout (0x04B0 RAT ticks = 300 us)
    (uint32_t)0x04B00243,
    // override_synth_disable_bias_div10.xml
    // Synth: Set divider bias to disabled
    HW32_ARRAY_OVERRIDE(0x405C,1),
    // Synth: Set divider bias to disabled (specific for loDivider=10)
    (uint32_t)0x18000280,
    // override_phy_rx_aaf_bw_0xd.xml
    // Rx: Set anti-aliasing filter bandwidth to 0xD (in ADI0, set IFAMPCTL3[7:4]=0xD)
    ADI_HALFREG_OVERRIDE(0,61,0xF,0xD),
    // override_phy_gfsk_rx.xml
    // Rx: Set LNA bias current trim offset to 3
    (uint32_t)0x00038883,
    // Rx: Freeze RSSI on sync found event
    HW_REG_OVERRIDE(0x6084,0x35F1),
    // override_phy_gfsk_pa_ramp_agc_reflevel_0x16.xml
    // Tx: Configure PA ramping setting (0x41). Rx: Set AGC reference level to 0x16.
    HW_REG_OVERRIDE(0x6088,0x4116),
    // Tx: Configure PA ramping setting
    HW_REG_OVERRIDE(0x608C,0x8213),
    // override_phy_long_range_dsss2.xml
    // PHY: Configure DSSS SF=2
    HW_REG_OVERRIDE(0x505C,0x0100),
    // override_phy_rx_rssi_offset_neg2db.xml
    // Rx: Set RSSI offset to adjust reported RSSI by -2 dB
    (uint32_t)0x000288A3,
    // TX power override
#if CCFG_FORCE_VDDR_HH
    // Tx: Set PA trim to max (in ADI0, set PACTL0=0xF8)
    ADI_REG_OVERRIDE(0,12,0xF8),
#endif
    (uint32_t)0x00000943, // Correct CRC initialization for 16-bit CRC
    (uint32_t)0x00000963, // Correct CRC initialization for 16-bit CRC
    (uint32_t)0xFFFFFFFF,
};

// Overrides for CMD_PROP_RADIO_DIV_SETUP
// US-EU Band, GFSK, no CC1190
static uint32_t pOverrides_fsk_200k_CC1350[] =
{
    // override_use_patch_prop_genfsk.xml
    // PHY: Use MCE ROM bank 4, RFE RAM patch
    MCE_RFE_OVERRIDE(0,4,0,1,0,0),
    // override_synth_prop_863_930_div5.xml
    // Synth: Set recommended RTRIM to 7
    HW_REG_OVERRIDE(0x4038,0x0037),
    // Synth: Set Fref to 4 MHz
    (uint32_t)0x000684A3,
    // Synth: Configure fine calibration setting
    HW_REG_OVERRIDE(0x4020,0x7F00),
    // Synth: Configure fine calibration setting
    HW_REG_OVERRIDE(0x4064,0x0040),
    // Synth: Configure fine calibration setting
    (uint32_t)0xB1070503,
    // Synth: Configure fine calibration setting
    (uint32_t)0x05330523,
    // Synth: Set loop bandwidth after lock to 20 kHz
    (uint32_t)0x0A480583,
    // Synth: Set loop bandwidth after lock to 20 kHz
    (uint32_t)0x7AB80603,
    // Synth: Configure VCO LDO (in ADI1, set VCOLDOCFG=0x9F to use voltage input reference)
    ADI_REG_OVERRIDE(1,4,0x9F),
    // Synth: Configure synth LDO (in ADI1, set SLDOCTL0.COMP_CAP=1)
    ADI_HALFREG_OVERRIDE(1,7,0x4,0x4),
    // Synth: Use 24 MHz XOSC as synth clock, enable extra PLL filtering
    (uint32_t)0x02010403,
    // Synth: Configure extra PLL filtering
    (uint32_t)0x00108463,
    // Synth: Increase synth programming timeout (0x04B0 RAT ticks = 300 us)
    (uint32_t)0x04B00243,
    // override_synth_disable_bias_div5.xml
    // Synth: Set divider bias to disabled
    HW32_ARRAY_OVERRIDE(0x405C,1),
    // Synth: Set divider bias to disabled (specific for loDivider=5)
    (uint32_t)0x18000200,
    // override_phy_rx_aaf_bw_0x0.xml
    // Rx: Set anti-aliasing filter bandwidth to 0x0 (in ADI0, set IFAMPCTL3[7:4]=0x0)
    ADI_HALFREG_OVERRIDE(0,61,0xF,0x0),
    // override_phy_gfsk_rx.xml
    // Rx: Set LNA bias current trim offset to 3
    (uint32_t)0x00038883,
    // Rx: Freeze RSSI on sync found event
    HW_REG_OVERRIDE(0x6084,0x35F1),
    // override_phy_gfsk_pa_ramp_5us_agc_reflevel_0x1c.xml
    // Tx: Configure PA ramping setting (0x10) for approximately 5 us PA ramp time. Rx: Set AGC reference level to 0x1C.
    HW_REG_OVERRIDE(0x6088,0x101C),
    // Tx: Configure PA ramping setting (0x08) for approximately 5 us PA ramp time
    HW_REG_OVERRIDE(0x608C,0x0813),
    // override_phy_rx_rssi_offset_5db.xml
    // Rx: Set RSSI offset to adjust reported RSSI by +5 dB
    (uint32_t)0x00FB88A3,
    // TX power override
#if CCFG_FORCE_VDDR_HH
    // Tx: Set PA trim to max (in ADI0, set PACTL0=0xF8)
    ADI_REG_OVERRIDE(0,12,0xF8),
#endif
    (uint32_t)0x00000943, // Correct CRC initialization for 16-bit CRC
    (uint32_t)0x00000963, // Correct CRC initialization for 16-bit CRC
    (uint32_t)0xFFFFFFFF,
};

// Overrides for CMD_PROP_RADIO_DIV_SETUP
static uint32_t pOverrides_fsk_200k_CC1310_CC1190[] =
{
    // override_use_patch_prop_genfsk.xml
    // PHY: Use MCE ROM bank 4, RFE RAM patch
    MCE_RFE_OVERRIDE(0,4,0,1,0,0),
    // override_synth_prop_863_930_div5.xml
    // Synth: Set recommended RTRIM to 7
    HW_REG_OVERRIDE(0x4038,0x0037),
    // Synth: Set Fref to 4 MHz
    (uint32_t)0x000684A3,
    // Synth: Configure fine calibration setting
    HW_REG_OVERRIDE(0x4020,0x7F00),
    // Synth: Configure fine calibration setting
    HW_REG_OVERRIDE(0x4064,0x0040),
    // Synth: Configure fine calibration setting
    (uint32_t)0xB1070503,
    // Synth: Configure fine calibration setting
    (uint32_t)0x05330523,
    // Synth: Set loop bandwidth after lock to 20 kHz
    (uint32_t)0x0A480583,
    // Synth: Set loop bandwidth after lock to 20 kHz
    (uint32_t)0x7AB80603,
    // Synth: Configure VCO LDO (in ADI1, set VCOLDOCFG=0x9F to use voltage input reference)
    ADI_REG_OVERRIDE(1,4,0x9F),
    // Synth: Configure synth LDO (in ADI1, set SLDOCTL0.COMP_CAP=1)
    ADI_HALFREG_OVERRIDE(1,7,0x4,0x4),
    // Synth: Use 24 MHz XOSC as synth clock, enable extra PLL filtering
    (uint32_t)0x02010403,
    // Synth: Configure extra PLL filtering
    (uint32_t)0x00108463,
    // Synth: Increase synth programming timeout (0x04B0 RAT ticks = 300 us)
    (uint32_t)0x04B00243,
    // override_synth_disable_bias_div5.xml
    // Synth: Set divider bias to disabled
    HW32_ARRAY_OVERRIDE(0x405C,1),
    // Synth: Set divider bias to disabled (specific for loDivider=5)
    (uint32_t)0x18000200,
    // override_phy_rx_aaf_bw_0x0.xml
    // Rx: Set anti-aliasing filter bandwidth to 0x0 (in ADI0, set IFAMPCTL3[7:4]=0x0)
    ADI_HALFREG_OVERRIDE(0,61,0xF,0x0),
    // override_phy_gfsk_rx.xml
    // Rx: Set LNA bias current trim offset to 3
    (uint32_t)0x00038883,
    // Rx: Freeze RSSI on sync found event
    HW_REG_OVERRIDE(0x6084,0x35F1),
    // override_phy_gfsk_pa_ramp_5us_agc_reflevel_0x1c.xml
    // Tx: Configure PA ramping setting (0x10) for approximately 5 us PA ramp time. Rx: Set AGC reference level to 0x1C.
    HW_REG_OVERRIDE(0x6088,0x101C),
    // Tx: Configure PA ramping setting (0x08) for approximately 5 us PA ramp time
    HW_REG_OVERRIDE(0x608C,0x0813),
    // override_phy_rx_rssi_offset_cc1310_cc1190_869.xml
    // Rx: Set RSSI offset to adjust reported RSSI by +26 dB
    (uint32_t)0x000188A3,
    // TX power override
    // Tx: Set PA trim to max (in ADI0, set PACTL0=0xF8)
    ADI_REG_OVERRIDE(0,12,0xF8),
    (uint32_t)0x00000943, // Correct CRC initialization for 16-bit CRC
    (uint32_t)0x00000963, // Correct CRC initialization for 16-bit CRC
    (uint32_t)0xFFFFFFFF,
};

static uint32_t pOverrides_ieee[] =
{
     0x00354038, /* Synth: Set RTRIM (POTAILRESTRIM) to 5 */
     0x4001402D, /* Synth: Correct CKVD latency setting (address) */
     0x00608402, /* Synth: Correct CKVD latency setting (value) */
     0x4001405D, /* Synth: Set ANADIV DIV_BIAS_MODE to PG1 (address) */
     0x1801F800, /* Synth: Set ANADIV DIV_BIAS_MODE to PG1 (value) */
     0x000784A3, /* Synth: Set FREF = 3.43 MHz (24 MHz / 7) */
     0xA47E0583, /* Synth: Set loop bandwidth after lock to 80 kHz (K2) */
     0xEAE00603, /* Synth: Set loop bandwidth after lock to 80 kHz (K3, LSB) */
     0x00010623, /* Synth: Set loop bandwidth after lock to 80 kHz (K3, MSB) */
     0x000288A3, /* ID: Adjust RSSI offset by 2 dB */
     0x000F8883, /* XD and ID: Force LNA IB to maximum */
     0x002B50DC, /* Adjust AGC DC filter */
     0x05000243, /* Increase synth programming timeout */
     0x002082C3, /* Set Rx FIFO threshold to avoid overflow */
     0x00018063, /* Disable pointer check */
     0xFFFFFFFF  /* End of override list */
};

/* US BAND and EURO BAND */
// Tx Power Values (Pout, TC, GC, IB)
// 1st column is for CC1310 and 2nd column is for CC1350
// This table can apply to both EM and Launch Pad
const txPwrVal_t txPowerTable_subg_CC1310[] =
{
    {-10, RF_TxPowerTable_DEFAULT_PA_ENTRY(0, 3, 0, 4) },
    {0, RF_TxPowerTable_DEFAULT_PA_ENTRY(1, 1, 0, 0) },
    {1, RF_TxPowerTable_DEFAULT_PA_ENTRY(3, 3, 0, 8) },
    {2, RF_TxPowerTable_DEFAULT_PA_ENTRY(2, 1, 0, 8) },
    {3, RF_TxPowerTable_DEFAULT_PA_ENTRY(4, 3, 0, 10) },
    {4, RF_TxPowerTable_DEFAULT_PA_ENTRY(5, 3, 0, 12) },
    {5, RF_TxPowerTable_DEFAULT_PA_ENTRY(6, 3, 0, 12) },
    {6, RF_TxPowerTable_DEFAULT_PA_ENTRY(7, 3, 0, 14) },
    {7, RF_TxPowerTable_DEFAULT_PA_ENTRY(9, 3, 0, 16) },
    {8, RF_TxPowerTable_DEFAULT_PA_ENTRY(11, 3, 0, 18) },
    {9, RF_TxPowerTable_DEFAULT_PA_ENTRY(13, 3, 0, 22) },
    {10, RF_TxPowerTable_DEFAULT_PA_ENTRY(19, 3, 0, 28) },
    {11, RF_TxPowerTable_DEFAULT_PA_ENTRY(26, 3, 0, 40) },
    {12, RF_TxPowerTable_DEFAULT_PA_ENTRY(24, 0, 0, 92) },
    {13, RF_TxPowerTable_DEFAULT_PA_ENTRY(63, 0, 0, 83) }, // The original PA value (12.5 dBm) have been rounded to an integer value.
    {14, RF_TxPowerTable_DEFAULT_PA_ENTRY(63, 0, 1, 83) }, // This setting requires CCFG_FORCE_VDDR_HH = 1.
    RF_TxPowerTable_TERMINATION_ENTRY
};

const txPwrVal_t txPowerTable_subg_CC1350[] =
{
    {-10, RF_TxPowerTable_DEFAULT_PA_ENTRY(0, 3, 0, 2) },
    {0, RF_TxPowerTable_DEFAULT_PA_ENTRY(3, 3, 0, 9) },
    {1, RF_TxPowerTable_DEFAULT_PA_ENTRY(4, 3, 0, 11) },
    {2, RF_TxPowerTable_DEFAULT_PA_ENTRY(5, 3, 0, 12) },
    {3, RF_TxPowerTable_DEFAULT_PA_ENTRY(6, 3, 0, 14) },
    {4, RF_TxPowerTable_DEFAULT_PA_ENTRY(4, 1, 0, 12) },
    {5, RF_TxPowerTable_DEFAULT_PA_ENTRY(8, 3, 0, 16) },
    {6, RF_TxPowerTable_DEFAULT_PA_ENTRY(9, 3, 0, 18) },
    {7, RF_TxPowerTable_DEFAULT_PA_ENTRY(11, 3, 0, 21) },
    {8, RF_TxPowerTable_DEFAULT_PA_ENTRY(14, 3, 0, 25) },
    {9, RF_TxPowerTable_DEFAULT_PA_ENTRY(18, 3, 0, 32) },
    {10, RF_TxPowerTable_DEFAULT_PA_ENTRY(24, 3, 0, 44) },
    {11, RF_TxPowerTable_DEFAULT_PA_ENTRY(37, 3, 0, 72) },
    {12, RF_TxPowerTable_DEFAULT_PA_ENTRY(43, 0, 0, 94) },
    {14, RF_TxPowerTable_DEFAULT_PA_ENTRY(63, 0, 1, 85) }, // This setting requires CCFG_FORCE_VDDR_HH = 1.
    RF_TxPowerTable_TERMINATION_ENTRY
};

const txPwrVal_t txPowerTable_subg_EU_CC1310_CC1190[] =
{
    {12, RF_TxPowerTable_DEFAULT_PA_ENTRY(0, 3, 0, 0) },
    {18, RF_TxPowerTable_DEFAULT_PA_ENTRY(1, 3, 0, 0) },
    {21, RF_TxPowerTable_DEFAULT_PA_ENTRY(2, 3, 0, 0) },
    {23, RF_TxPowerTable_DEFAULT_PA_ENTRY(3, 3, 0, 0) },
    {24, RF_TxPowerTable_DEFAULT_PA_ENTRY(4, 3, 0, 0) },
    {25, RF_TxPowerTable_DEFAULT_PA_ENTRY(6, 3, 0, 0) },
    {26, RF_TxPowerTable_DEFAULT_PA_ENTRY(11, 3, 0, 0) },
    RF_TxPowerTable_TERMINATION_ENTRY
};

const txPwrVal_t txPowerTable_subg_US_CC1310_CC1190[] =
{
    {7, RF_TxPowerTable_DEFAULT_PA_ENTRY(0, 3, 0, 0) },
    {14, RF_TxPowerTable_DEFAULT_PA_ENTRY(1, 3, 0, 0) },
    {18, RF_TxPowerTable_DEFAULT_PA_ENTRY(2, 3, 0, 0) },
    {20, RF_TxPowerTable_DEFAULT_PA_ENTRY(3, 3, 0, 0) },
    {22, RF_TxPowerTable_DEFAULT_PA_ENTRY(4, 3, 0, 0) },
    {23, RF_TxPowerTable_DEFAULT_PA_ENTRY(5, 3, 0, 0) },
    {24, RF_TxPowerTable_DEFAULT_PA_ENTRY(6, 3, 0, 0) },
    {25, RF_TxPowerTable_DEFAULT_PA_ENTRY(9, 3, 0, 0) },
    {26, RF_TxPowerTable_DEFAULT_PA_ENTRY(14, 3, 0, 0) },
    RF_TxPowerTable_TERMINATION_ENTRY
};

const txPwrVal_t txPowerTable_subg_433_CC1310[] =
{
    {6, RF_TxPowerTable_DEFAULT_PA_ENTRY(4, 3, 0, 17) },
    {10, RF_TxPowerTable_DEFAULT_PA_ENTRY(11, 3, 1, 30) },
    {13, RF_TxPowerTable_DEFAULT_PA_ENTRY(15, 0, 0, 53) },
    {14, RF_TxPowerTable_DEFAULT_PA_ENTRY(63, 0, 0, 95) }, // The original PA value (13.7 dBm) have been rounded to an integer value.
    {15, RF_TxPowerTable_DEFAULT_PA_ENTRY(63, 0, 0, 0) }, // This setting requires CCFG_FORCE_VDDR_HH = 1.
    RF_TxPowerTable_TERMINATION_ENTRY
};

const txPwrVal_t txPowerTable_subg_433_CC1350[] =
{
    {-10, RF_TxPowerTable_DEFAULT_PA_ENTRY(0, 3, 0, 2) },
    {0, RF_TxPowerTable_DEFAULT_PA_ENTRY(1, 3, 0, 7) },
    {2, RF_TxPowerTable_DEFAULT_PA_ENTRY(1, 3, 0, 9) },
    {3, RF_TxPowerTable_DEFAULT_PA_ENTRY(2, 3, 0, 11) },
    {4, RF_TxPowerTable_DEFAULT_PA_ENTRY(2, 3, 0, 12) },
    {5, RF_TxPowerTable_DEFAULT_PA_ENTRY(3, 3, 0, 16) },
    {6, RF_TxPowerTable_DEFAULT_PA_ENTRY(4, 3, 0, 18) },
    {7, RF_TxPowerTable_DEFAULT_PA_ENTRY(5, 3, 0, 21) },
    {8, RF_TxPowerTable_DEFAULT_PA_ENTRY(6, 3, 0, 23) },
    {9, RF_TxPowerTable_DEFAULT_PA_ENTRY(8, 3, 0, 28) },
    {10, RF_TxPowerTable_DEFAULT_PA_ENTRY(11, 3, 0, 35) },
    {11, RF_TxPowerTable_DEFAULT_PA_ENTRY(8, 1, 0, 39) },
    {12, RF_TxPowerTable_DEFAULT_PA_ENTRY(14, 1, 0, 60) },
    {13, RF_TxPowerTable_DEFAULT_PA_ENTRY(15, 0, 0, 108) },
    {14, RF_TxPowerTable_DEFAULT_PA_ENTRY(63, 0, 0, 92) }, // The original PA value (13.7 dBm) have been rounded to an integer value.
    {15, RF_TxPowerTable_DEFAULT_PA_ENTRY(63, 0, 1, 72) }, // This setting requires CCFG_FORCE_VDDR_HH = 1.
    RF_TxPowerTable_TERMINATION_ENTRY
};

const txPwrVal_t txPowerTable_ieee[] =
  { { TX_POWER_MINUS_21_DBM,        TX_POUT_DPA( 0x0D, 3, 0x08 ) },
    { TX_POWER_MINUS_18_DBM,        TX_POUT_DPA( 0x0D, 3, 0x0B ) },
    { TX_POWER_MINUS_15_DBM,        TX_POUT_DPA( 0x15, 3, 0x0E ) },
    { TX_POWER_MINUS_12_DBM,        TX_POUT_DPA( 0x19, 3, 0x14 ) },
    { TX_POWER_MINUS_9_DBM,         TX_POUT_DPA( 0x1D, 3, 0x1A ) },
    { TX_POWER_MINUS_6_DBM,         TX_POUT_DPA( 0x25, 3, 0x23 ) },
    { TX_POWER_MINUS_3_DBM,         TX_POUT_DPA( 0x2D, 3, 0x2F ) },
    { TX_POWER_0_DBM,               TX_POUT_DPA( 0x5B, 0, 0x1D ) },
    { TX_POWER_1_DBM,               TX_POUT_DPA( 0x63, 0, 0x21 ) },
    { TX_POWER_2_DBM,               TX_POUT_DPA( 0x6F, 0, 0x26 ) },
    { TX_POWER_3_DBM,               TX_POUT_DPA( 0x7F, 0, 0x2C ) },
    { TX_POWER_4_DBM,               TX_POUT_DPA( 0x77, 0, 0x34 ) },
    { TX_POWER_5_DBM,               TX_POUT_DPA( 0x5F, 0, 0x3C ) },
    { TX_POWER_6_DBM,               TX_POUT_DPA( 0x63, 0, 0x26 ) },
    { TX_POWER_7_DBM,               TX_POUT_DPA( 0x77, 0, 0x2E ) },
    { TX_POWER_8_DBM,               TX_POUT_DPA( 0x67, 0, 0x37 ) },
    { TX_POWER_9_DBM,               TX_POUT_DPA( 0x3D, 0, 0x3F ) },
      RF_TxPowerTable_TERMINATION_ENTRY };

#endif /* _CONFIG_CC13X0_RF_TABLE_H */
