/*
 * @brief WM8903 Audio codec interface file
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

#include "board.h"
#include "wm8903.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
#define I2S_MCK_PORT       1
#define I2S_MCK_PIN        17
#define I2S_MCK_MUX        2

#define I2S_SCK_PORT       0
#define I2S_SCK_PIN        4
#define I2S_SCK_MUX        1

#define I2S_WS_PORT        0
#define I2S_WS_PIN         5
#define I2S_WS_MUX         1


#define I2CDEV_CODEC_ADDR       0x1A
#define CODEC_I2C_BUS           LPC_I2C0
#define CODEC_I2C_BUS_ID        I2C0

#define MULTI_REGISTER_DELAY_COMMAND               0xFFFF
#define MULTI_REGISTER_WAIT_FOR_SEQUENCER_NOT_BUSY 0xFFFE

typedef struct __WM8903_Init_Seq {
	uint16_t reg_adr;
	uint16_t reg_val;
} WM8903_Init_Seq_t;

const WM8903_Init_Seq_t g_wm8903[] = {

	/* Note! This sequence assumes MCLK is supplied to CODEC. */

	/* execute default start=up sequence */
	{ WM8903_SW_RESET_AND_ID,   0x0000},
	{ WM8903_AUDIO_INTERFACE_1, 0x0002},      /* audio interface I2S 32-bit */
	{ WM8903_WRITE_SEQUENCER_0, 0x0100},      /* WSEQ_ENA=1, WSEQ_WRITE_INDEX=0_0000 */
	{ WM8903_CLOCK_RATES_2,     0x0004},      /* clock enable SYS */
	{ WM8903_WRITE_SEQUENCER_3, 0x0100},      /* WSEQ_ABORT=0, WSEQ_START=1, WSEQ_START_INDEX=00_0000 */

	/* Wait up to 4 seconds for sequencer not busy. */
	{ MULTI_REGISTER_WAIT_FOR_SEQUENCER_NOT_BUSY, 4000 },

	{ WM8903_CLOCK_RATES_2, 0x0006},          /* clock enable SYS and DSP */

	/* DAC setup */
	{ WM8903_DC_SERVO_0, 0x001c},             /* d.c servo enable on headphone outputs */
	{ WM8903_ANALOGUE_LINEOUT_0, 0x0000},     /* line out disabled */
	{ WM8903_POWER_MANAGEMENT_3, 0x0000},     /* disable line out PGAs */
	{ WM8903_CLASS_W_0, 0x0001},              /* dynamic CP control based on audio */

	/* Set clock to 256k extrn MCK */
	{ WM8903_CLOCK_RATES_1, 0x0C08},          /* 256x and external MCLK */

	/* Disable the DMIC */
	{ WM8903_CLOCK_RATE_TEST_4, (0<<9)},      /* DMIC off */

	/* Set L/R gain and unmute */
	{ WM8903_ANALOGUE_OUT1_LEFT, 0x0039},     /* HP left 0dB */
	{ WM8903_ANALOGUE_OUT1_RIGHT, 0x00b9},    /* HP right 0dB */
	{ WM8903_ANALOGUE_LEFT_INPUT_0, 0x0005},  /* Un-mute 0dB left */
	{ WM8903_ANALOGUE_RIGHT_INPUT_0, 0x0005}, /* Un-mute 0dB right */

	{ WM8903_GPIO_CONTROL_1, ((0x06 << 8) | (0 << 7))},
	{ WM8903_GPIO_CONTROL_2, ((0x06 << 8) | (1 << 7))},

};

/* I2CM transfer record */
static I2CM_XFER_T  i2cmXferRec;

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/


/*****************************************************************************
 * Private functions
 ****************************************************************************/

/* Write multiple registers in one go */
static int WM8903_MultiRegWrite(const WM8903_Init_Seq_t* seq, uint32_t cnt)
{
	uint32_t timeoutTimer;
	uint32_t timeoutTicks;
	uint16_t rd_val = 1;
	uint32_t i;

	for (i = 0; i < cnt; i++) {

		switch ( seq[i].reg_adr ) {
			case MULTI_REGISTER_DELAY_COMMAND:

				/* Delay the requested number of mS */
				StopWatch_DelayMs( seq[i].reg_val );
				break;

			case MULTI_REGISTER_WAIT_FOR_SEQUENCER_NOT_BUSY:

				/* Don't lockup if the write sequencer does. */
				timeoutTimer = StopWatch_Start();
				timeoutTicks = StopWatch_MsToTicks( seq[i].reg_val );

				/* Wait in this loop until write sequencer is not busy. */
				do { 

					if (StopWatch_Elapsed(timeoutTimer) > timeoutTicks) {
						return 0;
					}

					/* Read WSEQ_BUSY flag. */
					rd_val = WM8903_REG_Read(WM8903_WRITE_SEQUENCER_4);
				} while (rd_val & WSEQ_BUSY_MASK);
				break;

			default:
				WM8903_REG_Write(seq[i].reg_adr, seq[i].reg_val);
				break;
		}
	}
	return (SUCCESS);
}

/* Function to setup and execute I2C transfer request */
static void SetupXferRecAndExecute(uint8_t devAddr,
								   uint8_t *txBuffPtr,
								   uint16_t txSize,
								   uint8_t *rxBuffPtr,
								   uint16_t rxSize)
{
	/* Setup I2C transfer record */
    i2cmXferRec.slaveAddr = devAddr;
    i2cmXferRec.options = 0;
    i2cmXferRec.status = 0;
    i2cmXferRec.txSz = txSize;
    i2cmXferRec.rxSz = rxSize;
    i2cmXferRec.txBuff = txBuffPtr;
    i2cmXferRec.rxBuff = rxBuffPtr;
    Chip_I2CM_XferBlocking(CODEC_I2C_BUS, &i2cmXferRec);
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/* Read data from UDA register */
uint16_t WM8903_REG_Read(uint8_t reg) {
	uint8_t rx_data[2];

	SetupXferRecAndExecute(I2CDEV_CODEC_ADDR, &reg, 1, rx_data, 2);

	/* Test for valid operation */
	if (i2cmXferRec.status != I2CM_STATUS_OK) {
		return 0xFFFF;
	}

	return (rx_data[0] << 8) | rx_data[1];
}

/* Write data to Codec register */
uint32_t WM8903_REG_Write(uint8_t reg, uint16_t val)
{
	uint8_t dat[3];
	dat[0] = reg; dat[1] = val >> 8; dat[2] = val & 0xFF;

	SetupXferRecAndExecute(I2CDEV_CODEC_ADDR, dat, 3, NULL, 0);
    
    return i2cmXferRec.status;
}

/* WM8903 initialize function */
int WM8903_Init(int input)
{
	int ret;
	
	/* Initialize I2C to 100khz */
	Board_I2C_Init(CODEC_I2C_BUS_ID);
	Chip_I2CM_Init(CODEC_I2C_BUS);
	Chip_I2CM_SetBusSpeed(CODEC_I2C_BUS, 100000);
	
	/* Disable the interrupt for the I2C */
	NVIC_DisableIRQ(I2C0_IRQn);

	/* Disable all clocks to device. Configuring as gpio disables I2S functionality */
	Chip_IOCON_PinMuxSet(LPC_IOCON, I2S_MCK_PORT, I2S_MCK_PIN, 0x0);
	Chip_IOCON_PinMuxSet(LPC_IOCON, I2S_SCK_PORT, I2S_SCK_PIN, 0x0);
	Chip_IOCON_PinMuxSet(LPC_IOCON, I2S_WS_PORT, I2S_WS_PIN, 0x0);

	/* Send soft reset */
	WM8903_REG_Write(WM8903_SW_RESET_AND_ID, 0x0);

	/* Enable the master clock and give the part time to respond 2 milSeconds */
	Chip_IOCON_PinMuxSet(LPC_IOCON, I2S_MCK_PORT, I2S_MCK_PIN, 0x2);

	/* Delay for 2 mSec */
	StopWatch_DelayMs(2); 
	
	/* Initialize the default values */
	ret = WM8903_MultiRegWrite(g_wm8903, sizeof(g_wm8903) / sizeof(WM8903_Init_Seq_t));

	/* Lastly turn the other clocks back on */
	Chip_IOCON_PinMuxSet(LPC_IOCON, I2S_SCK_PORT, I2S_SCK_PIN, 0x1);
	Chip_IOCON_PinMuxSet(LPC_IOCON, I2S_WS_PORT, I2S_WS_PIN, 0x1);
    
	return ret;
}

void WM8903_DeInit(void)
{
	 Chip_I2CM_DeInit(CODEC_I2C_BUS);
}

typedef struct 
{
	char* name;
	uint16_t address;
} wm8903_register_lookup_table;

wm8903_register_lookup_table registerLookupTable[] =
{
	{ "SW_RESET_AND_ID",				WM8903_SW_RESET_AND_ID },
	{ "REVISION",						WM8903_REVISION },
	{ "BIAS_CONTROL_0",					WM8903_BIAS_CONTROL_0 },
	{ "VMID_CONTROL_0",					WM8903_VMID_CONTROL_0 },
	{ "MIC_BIAS_CONTROL_0",				WM8903_MIC_BIAS_CONTROL_0 },
	{ "ANALOGUE_DAC_0",					WM8903_ANALOGUE_DAC_0 },
	{ "ANALOGUE_ADC_0",					WM8903_ANALOGUE_ADC_0 },
	{ "POWER_MANAGEMENT_0",				WM8903_POWER_MANAGEMENT_0 },
	{ "POWER_MANAGEMENT_1",				WM8903_POWER_MANAGEMENT_1 },
	{ "POWER_MANAGEMENT_2",				WM8903_POWER_MANAGEMENT_2 },
	{ "POWER_MANAGEMENT_3",				WM8903_POWER_MANAGEMENT_3 },
	{ "POWER_MANAGEMENT_4",				WM8903_POWER_MANAGEMENT_4 },
	{ "POWER_MANAGEMENT_5",				WM8903_POWER_MANAGEMENT_5 },
	{ "POWER_MANAGEMENT_6",				WM8903_POWER_MANAGEMENT_6 },
	{ "CLOCK_RATES_0",					WM8903_CLOCK_RATES_0 },
	{ "CLOCK_RATES_1",					WM8903_CLOCK_RATES_1 },
	{ "CLOCK_RATES_2",					WM8903_CLOCK_RATES_2 },
	{ "AUDIO_INTERFACE_0",				WM8903_AUDIO_INTERFACE_0 },
	{ "AUDIO_INTERFACE_1",				WM8903_AUDIO_INTERFACE_1 },
	{ "AUDIO_INTERFACE_2",				WM8903_AUDIO_INTERFACE_2 },
	{ "AUDIO_INTERFACE_3",				WM8903_AUDIO_INTERFACE_3 },
	{ "DAC_DIGITAL_VOLUME_LEFT",		WM8903_DAC_DIGITAL_VOLUME_LEFT },
	{ "DAC_DIGITAL_VOLUME_RIGHT",		WM8903_DAC_DIGITAL_VOLUME_RIGHT },
	{ "DAC_DIGITAL_0",					WM8903_DAC_DIGITAL_0 },
	{ "DAC_DIGITAL_1",					WM8903_DAC_DIGITAL_1 },
	{ "ADC_DIGITAL_VOLUME_LEFT",		WM8903_ADC_DIGITAL_VOLUME_LEFT },
	{ "ADC_DIGITAL_VOLUME_RIGHT",		WM8903_ADC_DIGITAL_VOLUME_RIGHT },
	{ "ADC_DIGITAL_0",					WM8903_ADC_DIGITAL_0 },
	{ "DRC_0",							WM8903_DRC_0 },
	{ "DRC_1",							WM8903_DRC_1 },
	{ "DRC_2",							WM8903_DRC_2 },
	{ "DRC_3",							WM8903_DRC_3 },
	{ "ANALOGUE_LEFT_INPUT_0",			WM8903_ANALOGUE_LEFT_INPUT_0 },
	{ "ANALOGUE_RIGHT_INPUT_0",			WM8903_ANALOGUE_RIGHT_INPUT_0 },
	{ "ANALOGUE_LEFT_INPUT_1",			WM8903_ANALOGUE_LEFT_INPUT_1 },
	{ "ANALOGUE_RIGHT_INPUT_1",			WM8903_ANALOGUE_RIGHT_INPUT_1 },
	{ "ANALOGUE_LEFT_MIX_0",			WM8903_ANALOGUE_LEFT_MIX_0 },
	{ "ANALOGUE_RIGHT_MIX_0",			WM8903_ANALOGUE_RIGHT_MIX_0 },
	{ "ANALOGUE_SPK_MIX_LEFT_0",		WM8903_ANALOGUE_SPK_MIX_LEFT_0 },
	{ "ANALOGUE_SPK_MIX_LEFT_1",		WM8903_ANALOGUE_SPK_MIX_LEFT_1 },
	{ "ANALOGUE_SPK_MIX_RIGHT_0",		WM8903_ANALOGUE_SPK_MIX_RIGHT_0 },
	{ "ANALOGUE_SPK_MIX_RIGHT_1",		WM8903_ANALOGUE_SPK_MIX_RIGHT_1 },
	{ "ANALOGUE_OUT1_LEFT",				WM8903_ANALOGUE_OUT1_LEFT },
	{ "ANALOGUE_OUT1_RIGHT",			WM8903_ANALOGUE_OUT1_RIGHT },
	{ "ANALOGUE_OUT2_LEFT",				WM8903_ANALOGUE_OUT2_LEFT },
	{ "ANALOGUE_OUT2_RIGHT",			WM8903_ANALOGUE_OUT2_RIGHT },
	{ "ANALOGUE_OUT3_LEFT",				WM8903_ANALOGUE_OUT3_LEFT },
	{ "ANALOGUE_OUT3_RIGHT",			WM8903_ANALOGUE_OUT3_RIGHT },
	{ "ANALOGUE_SPK_OUTPUT_CONTROL_0",	WM8903_ANALOGUE_SPK_OUTPUT_CONTROL_0 },
	{ "DC_SERVO_0",						WM8903_DC_SERVO_0 },
	{ "DC_SERVO_2",						WM8903_DC_SERVO_2 },
	{ "DC_SERVO_4",						WM8903_DC_SERVO_4 },
	{ "DC_SERVO_5",						WM8903_DC_SERVO_5 },
	{ "DC_SERVO_6",						WM8903_DC_SERVO_6 },
	{ "DC_SERVO_7",						WM8903_DC_SERVO_7 },
	{ "DC_SERVO_READBACK_1",			WM8903_DC_SERVO_READBACK_1 },
	{ "DC_SERVO_READBACK_2",			WM8903_DC_SERVO_READBACK_2 },
	{ "DC_SERVO_READBACK_3",			WM8903_DC_SERVO_READBACK_3 },
	{ "DC_SERVO_READBACK_4",			WM8903_DC_SERVO_READBACK_4 },
	{ "ANALOGUE_HP_0",					WM8903_ANALOGUE_HP_0 },
	{ "ANALOGUE_LINEOUT_0",				WM8903_ANALOGUE_LINEOUT_0 },
	{ "CHARGE_PUMP_0",					WM8903_CHARGE_PUMP_0 },
	{ "CLASS_W_0",						WM8903_CLASS_W_0 },
	{ "WRITE_SEQUENCER_0",				WM8903_WRITE_SEQUENCER_0 },
	{ "WRITE_SEQUENCER_1",				WM8903_WRITE_SEQUENCER_1 },
	{ "WRITE_SEQUENCER_2",				WM8903_WRITE_SEQUENCER_2 },
	{ "WRITE_SEQUENCER_3",				WM8903_WRITE_SEQUENCER_3 },
	{ "WRITE_SEQUENCER_4",				WM8903_WRITE_SEQUENCER_4 },
	{ "GPIO_CONTROL_1",					WM8903_GPIO_CONTROL_1 },
	{ "GPIO_CONTROL_2",					WM8903_GPIO_CONTROL_2 },
	{ "GPIO_CONTROL_3",					WM8903_GPIO_CONTROL_3 },
	{ "GPIO_CONTROL_4",					WM8903_GPIO_CONTROL_4 },
	{ "GPIO_CONTROL_5",					WM8903_GPIO_CONTROL_5 },
	{ "INTERRUPT_STATUS",				WM8903_INTERRUPT_STATUS },
	{ "INTERRUPT_STATUS_MASK",			WM8903_INTERRUPT_STATUS_MASK },
	{ "INTERRUPT_POLARITY_1",			WM8903_INTERRUPT_POLARITY_1 },
	{ "INTERRUPT_CONTROL",				WM8903_INTERRUPT_CONTROL },
	{ "FLL_CONTROL_1",					WM8903_FLL_CONTROL_1 },
	{ "FLL_CONTROL_2",					WM8903_FLL_CONTROL_2 },
	{ "FLL_CONTROL_3",					WM8903_FLL_CONTROL_3 },
	{ "FLL_CONTROL_4",					WM8903_FLL_CONTROL_4 },
	{ "CLOCK_RATE_TEST_4",				WM8903_CLOCK_RATE_TEST_4 },
	{ "ANALOGUE_OUTPUT_BIAS_0",			WM8903_ANALOGUE_OUTPUT_BIAS_0 },
	{ "ANALOGUE_OUTPUT_BIAS_2",			WM8903_ANALOGUE_OUTPUT_BIAS_2 },
	/* Last entry has NULL to mark the end */
	{ NULL, 0 }
};

void WM8903_DumpAllRegisters(void)
{
	uint32_t ix;
	DEBUGOUT( "Dump of all registers:\r\n" );

	/* Iterate through the list of registers... Exit loop when end is reached. */
	for (ix=0; registerLookupTable[ix].name;ix++) {

		uint16_t valueRead = WM8903_REG_Read( registerLookupTable[ix].address );

		DEBUGOUT( "%-30s = 0x%04X\r\n", registerLookupTable[ix].name, valueRead );
	}
}


/**
 * @}
 */
