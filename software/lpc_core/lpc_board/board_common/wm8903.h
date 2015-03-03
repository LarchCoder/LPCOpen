/*
 * @brief WM8903 Audio Codec header
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2014
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#ifndef _WM8903_H
#define _WM8903_H

/** @defgroup BOARD_COMMON_WM8903 BOARD: WM8903 Audio codec interface module
 * @ingroup BOARD_Common
 * WM8903 Audio codec interface module, the module registers are accessed
 * using I2C. The board which uses this module must define WM8903_I2C_BUS to I2C0,
 * I2C1, etc, based on which I2C bus is connected to WM8903. All the
 * functions in this modules assumes that the I2C interrupt for WM8903_I2C_BUS
 * is enabled and Chip_I2C_MasterStateHandler(WM8903_I2C_BUS) is called from the
 * ISR. If the functions are to be used in polling mode the caller must replace
 * the event handler to Chip_I2C_EventHandlerPolling(), by using API
 * Chip_I2C_SetMasterEventHandler(). A macro I2CDEV_WM8903_ADDR must be defined
 * to the appropriate slave address of WM8903 audio codec.
 * @{
 */

#define WM8903_CLK_MCLK                         1
#define WM8903_CLK_FLL                          2

#define WM8903_FLL_MCLK                         1
#define WM8903_FLL_BCLK                         2
#define WM8903_FLL_LRCLK                        3
#define WM8903_FLL_FREE_RUNNING                 4

/*
 * Register values.
 */
#define WM8903_SW_RESET_AND_ID                  0x00
#define WM8903_REVISION                         0x01
#define WM8903_BIAS_CONTROL_0                   0x04
#define WM8903_VMID_CONTROL_0                   0x05
#define WM8903_MIC_BIAS_CONTROL_0               0x06
#define WM8903_ANALOGUE_DAC_0                   0x08
#define WM8903_ANALOGUE_ADC_0                   0x0A
#define WM8903_POWER_MANAGEMENT_0               0x0C
#define WM8903_POWER_MANAGEMENT_1               0x0D
#define WM8903_POWER_MANAGEMENT_2               0x0E
#define WM8903_POWER_MANAGEMENT_3               0x0F
#define WM8903_POWER_MANAGEMENT_4               0x10
#define WM8903_POWER_MANAGEMENT_5               0x11
#define WM8903_POWER_MANAGEMENT_6               0x12
#define WM8903_CLOCK_RATES_0                    0x14
#define WM8903_CLOCK_RATES_1                    0x15
#define WM8903_CLOCK_RATES_2                    0x16
#define WM8903_AUDIO_INTERFACE_0                0x18
#define WM8903_AUDIO_INTERFACE_1                0x19
#define WM8903_AUDIO_INTERFACE_2                0x1A
#define WM8903_AUDIO_INTERFACE_3                0x1B
#define WM8903_DAC_DIGITAL_VOLUME_LEFT          0x1E
#define WM8903_DAC_DIGITAL_VOLUME_RIGHT         0x1F
#define WM8903_DAC_DIGITAL_0                    0x20
#define WM8903_DAC_DIGITAL_1                    0x21
#define WM8903_ADC_DIGITAL_VOLUME_LEFT          0x24
#define WM8903_ADC_DIGITAL_VOLUME_RIGHT         0x25
#define WM8903_ADC_DIGITAL_0                    0x26
#define WM8903_DRC_0                            0x28
#define WM8903_DRC_1                            0x29
#define WM8903_DRC_2                            0x2A
#define WM8903_DRC_3                            0x2B
#define WM8903_ANALOGUE_LEFT_INPUT_0            0x2C
#define WM8903_ANALOGUE_RIGHT_INPUT_0           0x2D
#define WM8903_ANALOGUE_LEFT_INPUT_1            0x2E
#define WM8903_ANALOGUE_RIGHT_INPUT_1           0x2F
#define WM8903_ANALOGUE_LEFT_MIX_0              0x32
#define WM8903_ANALOGUE_RIGHT_MIX_0             0x33
#define WM8903_ANALOGUE_SPK_MIX_LEFT_0          0x34
#define WM8903_ANALOGUE_SPK_MIX_LEFT_1          0x35
#define WM8903_ANALOGUE_SPK_MIX_RIGHT_0         0x36
#define WM8903_ANALOGUE_SPK_MIX_RIGHT_1         0x37
#define WM8903_ANALOGUE_OUT1_LEFT               0x39
#define WM8903_ANALOGUE_OUT1_RIGHT              0x3A
#define WM8903_ANALOGUE_OUT2_LEFT               0x3B
#define WM8903_ANALOGUE_OUT2_RIGHT              0x3C
#define WM8903_ANALOGUE_OUT3_LEFT               0x3E
#define WM8903_ANALOGUE_OUT3_RIGHT              0x3F
#define WM8903_ANALOGUE_SPK_OUTPUT_CONTROL_0    0x41
#define WM8903_DC_SERVO_0                       0x43
#define WM8903_DC_SERVO_2                       0x45
#define WM8903_DC_SERVO_4                       0x47
#define WM8903_DC_SERVO_5                       0x48
#define WM8903_DC_SERVO_6                       0x49
#define WM8903_DC_SERVO_7                       0x4A
#define WM8903_DC_SERVO_READBACK_1              0x51
#define WM8903_DC_SERVO_READBACK_2              0x52
#define WM8903_DC_SERVO_READBACK_3              0x53
#define WM8903_DC_SERVO_READBACK_4              0x54
#define WM8903_ANALOGUE_HP_0                    0x5A
#define WM8903_ANALOGUE_LINEOUT_0               0x5E
#define WM8903_CHARGE_PUMP_0                    0x62
#define WM8903_CLASS_W_0                        0x68
#define WM8903_WRITE_SEQUENCER_0                0x6C
#define WM8903_WRITE_SEQUENCER_1                0x6D
#define WM8903_WRITE_SEQUENCER_2                0x6E
#define WM8903_WRITE_SEQUENCER_3                0x6F
#define WM8903_WRITE_SEQUENCER_4                0x70
#define WM8903_GPIO_CONTROL_1                   0x74
#define WM8903_GPIO_CONTROL_2                   0x75
#define WM8903_GPIO_CONTROL_3                   0x76
#define WM8903_GPIO_CONTROL_4                   0x77
#define WM8903_GPIO_CONTROL_5                   0x78
#define WM8903_INTERRUPT_STATUS                 0x79
#define WM8903_INTERRUPT_STATUS_MASK            0x7A
#define WM8903_INTERRUPT_POLARITY_1             0x7B
#define WM8903_INTERRUPT_CONTROL                0x7E
#define WM8903_FLL_CONTROL_1                    0x80
#define WM8903_FLL_CONTROL_2                    0x81
#define WM8903_FLL_CONTROL_3                    0x82
#define WM8903_FLL_CONTROL_4                    0x83
#define WM8903_CLOCK_RATE_TEST_4                0xA4
#define WM8903_ANALOGUE_OUTPUT_BIAS_0           0xAC
#define WM8903_ANALOGUE_OUTPUT_BIAS_2           0xBB

#define WM8903_REGISTER_COUNT                   84
#define WM8903_MAX_REGISTER                     0xBB

/* Some bit definitions in the above registers. */
#define WSEQ_BUSY_BIT                           0
#define WSEQ_BUSY_MASK                          (1<<WSEQ_BUSY_BIT)
#define FLL_LOCK_EINT_BIT                       5
#define FLL_LOCK_EINT_MASK                      (1<<FLL_LOCK_EINT_BIT)


/* register setting constants */
#define WM8903_FLL_1288MHZ_K                    (0x3127)
#define WM8903_FLL_1288MHZ_N                    (0x8 << 5)

/* DAC_MONO=0, DAC_SB_FILT=0, DAC_MUTERATE=0, DAC_UNMUTE_RAMP=1, DAC_OSR128=1, DAC_MUTE=0, DEEMPH=00 */
#define WM8903_DAC_DIGITAL_1_VALUE              0x0240


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief	Write a 16-bit value to Codec Register
 * @param	reg		: Register to which @a val be written
 * @param	val		: 16-Bit value to be written
 * @return	Status of I2C write.
 */
uint32_t WM8903_REG_Write(uint8_t reg, uint16_t val);

/**
 * @brief	Read a 16-bit value from WM8903 codec register
 * @param	reg		: Register from which the value to be read
 * @return	Returns the value read from the register
 */
uint16_t WM8903_REG_Read(uint8_t reg);

/**
 * @brief	Initialize WM8903 to its default state
 * @param	input	: Audio input source (Must be one of  WM8903_LINE_IN
 *                    or WM8903_MIC_IN_L or WM8903_MIC_IN_LR)
 * @return	1 on Success and 0 on failure
 */
int WM8903_Init(int input);

/**
 * @brief	Disable the WM8903 
 * @return	Nothing
 */
void WM8903_DeInit(void);

/**
 * @brief	Debug function to print contents of all WM8903 registers to debug console
 * @return	Nothing
 */
void WM8903_DumpAllRegisters(void);


#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* _WM8903__ */

