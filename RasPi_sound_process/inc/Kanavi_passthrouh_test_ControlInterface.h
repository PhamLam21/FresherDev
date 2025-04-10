/****************************************************************************
 *
 *		Target Tuning Symbol File
 *		-------------------------
 *
 * 		This file is populated with symbol information only for modules
 *		that have an object ID of 30000 or greater assigned.
 *
 *          Generated on:  09-Apr-2025 14:28:02
 *
 ***************************************************************************/

#ifndef AWE_KANAVI_PASSTHROUH_TEST_CONTROLINTERFACE_H
#define AWE_KANAVI_PASSTHROUH_TEST_CONTROLINTERFACE_H

// ----------------------------------------------------------------------
//  [ScalerNV2]
// Newly created subsystem

#define AWE_ScalerN2_classID 0xBEEF0814
#define AWE_ScalerN2_ID 30001

// int profileTime - 24.8 fixed point filtered execution time. Must be pumped 1000 times to get to within .1% accuracy
#define AWE_ScalerN2_profileTime_HANDLE 0x07531007
#define AWE_ScalerN2_profileTime_MASK 0x00000080
#define AWE_ScalerN2_profileTime_SIZE 0x00000001

// float masterGain - Overall gain to apply.
// Default value: -20
// Range: -60 to 24
#define AWE_ScalerN2_masterGain_HANDLE 0x07531008
#define AWE_ScalerN2_masterGain_MASK 0x00000100
#define AWE_ScalerN2_masterGain_SIZE 0x00000001

// float smoothingTime - Time constant of the smoothing process (0 = 
//         unsmoothed).
// Default value: 10
// Range: 0 to 1000
#define AWE_ScalerN2_smoothingTime_HANDLE 0x07531009
#define AWE_ScalerN2_smoothingTime_MASK 0x00000200
#define AWE_ScalerN2_smoothingTime_SIZE 0x00000001

// int isDB - Selects between linear (=0) and dB (=1) operation
// Default value: 1
// Range: 0 to 1
#define AWE_ScalerN2_isDB_HANDLE 0x0753100A
#define AWE_ScalerN2_isDB_MASK 0x00000400
#define AWE_ScalerN2_isDB_SIZE 0x00000001

// float smoothingCoeff - Smoothing coefficient.
#define AWE_ScalerN2_smoothingCoeff_HANDLE 0x0753100B
#define AWE_ScalerN2_smoothingCoeff_MASK 0x00000800
#define AWE_ScalerN2_smoothingCoeff_SIZE 0x00000001

// float trimGain[4] - Array of trim gains, one per channel
// Default value:
//     0
//     0
//     0
//     0
// Range: -24 to 24
#define AWE_ScalerN2_trimGain_HANDLE 0x8753100C
#define AWE_ScalerN2_trimGain_MASK 0x00001000
#define AWE_ScalerN2_trimGain_SIZE 0x00000004

// float targetGain[4] - Computed target gains in linear units
#define AWE_ScalerN2_targetGain_HANDLE 0x8753100D
#define AWE_ScalerN2_targetGain_MASK 0x00002000
#define AWE_ScalerN2_targetGain_SIZE 0x00000004

// float currentGain[4] - Instanteous gains.  These ramp towards 
//         targetGain
#define AWE_ScalerN2_currentGain_HANDLE 0x8753100E
#define AWE_ScalerN2_currentGain_MASK 0x00004000
#define AWE_ScalerN2_currentGain_SIZE 0x00000004


// ----------------------------------------------------------------------
//  [MuteSmoothed]
// Newly created subsystem

#define AWE_Mute1_classID 0xBEEF081C
#define AWE_Mute1_ID 30002

// int profileTime - 24.8 fixed point filtered execution time. Must be pumped 1000 times to get to within .1% accuracy
#define AWE_Mute1_profileTime_HANDLE 0x07532007
#define AWE_Mute1_profileTime_MASK 0x00000080
#define AWE_Mute1_profileTime_SIZE 0x00000001

// int isMuted - Boolean that controls muting/unmuting.
// Default value: 0
// Range: 0 to 1
#define AWE_Mute1_isMuted_HANDLE 0x07532008
#define AWE_Mute1_isMuted_MASK 0x00000100
#define AWE_Mute1_isMuted_SIZE 0x00000001

// float smoothingTime - Time constant of the smoothing process
// Default value: 10
// Range: 0 to 1000
#define AWE_Mute1_smoothingTime_HANDLE 0x07532009
#define AWE_Mute1_smoothingTime_MASK 0x00000200
#define AWE_Mute1_smoothingTime_SIZE 0x00000001

// float currentGain - Instantaneous gain applied by the module.  This 
//         is also the starting gain of the module.
#define AWE_Mute1_currentGain_HANDLE 0x0753200A
#define AWE_Mute1_currentGain_MASK 0x00000400
#define AWE_Mute1_currentGain_SIZE 0x00000001

// float smoothingCoeff - Smoothing coefficient.
#define AWE_Mute1_smoothingCoeff_HANDLE 0x0753200B
#define AWE_Mute1_smoothingCoeff_MASK 0x00000800
#define AWE_Mute1_smoothingCoeff_SIZE 0x00000001

// float gain - Target gain.
#define AWE_Mute1_gain_HANDLE 0x0753200C
#define AWE_Mute1_gain_MASK 0x00001000
#define AWE_Mute1_gain_SIZE 0x00000001


#define AWE_OBJECT_FOUND 0

#endif // AWE_KANAVI_PASSTHROUH_TEST_CONTROLINTERFACE_H

