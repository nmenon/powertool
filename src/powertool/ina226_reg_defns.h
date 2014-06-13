/*
 * INA226 Register definitions.
 * For more information see http://www.ti.com/product/ina226
 *
 * Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
 *	Nishanth Menon
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#ifndef __INA226_REG_DEFNS__
#define __INA226_REG_DEFNS__

#include <proj-types.h>

/* INA226 register offsets 8 bit addressing, 16 bit data */
#define REG_CONFIG	0x0
#define REG_SHUNT	0x1
#define REG_BUS		0x2
#define REG_POWER	0x3
#define REG_CURRENT	0x4
#define REG_CAL		0x5
#define REG_IRQ_ENABLE	0x6
#define REG_DIEID	0xFF

/* == REG_CONFIG definition == */
#define	REG_CONFIG_MODE_MASK	(0x7 << 0)
#define CONFIG_MODE_TRIGGER(MODE)	((MODE) | 0)
#define CONFIG_MODE_CONTINOUS(MODE)	((MODE) | 0x4 )
#define CONFIG_MODE_V_SHUNT_VOLT	0x1
#define CONFIG_MODE_V_BUS_VOLT		0x2

/* Valid conversion times for VBUS CT: and VSH CT */
#define	REG_CONFIG_VSH_CT_MASK	(0x7 << 3)
#define	REG_CONFIG_VBUS_CT_MASK	(0x7 << 6)

#ifdef CONVERT_VALUES
/* Time in uSeconds */
static unsigned int config_mode_conversion_time[] = {
	[0] = 140,
	[1] = 204,
	[2] = 332,
	[3] = 588,
	[4] = 1100,
	[5] = 2116,
	[6] = 4156,
	[7] = 8224,
};

/* Averaging */
static unsigned int config_mode_average_numer[] = {
	[0] = 1,
	[1] = 4,
	[2] = 16,
	[3] = 64,
	[4] = 128,
	[5] = 256,
	[6] = 512,
	[7] = 1024,
};
#endif

#define REG_CONFIG_AVG_MASK	(0x7 << 9)
#define REG_CONFIG_RESET	(0x1 << 15)

/* == REG_SHUNT definition (signed value) == */
#define REG_SHUNT_MASK		(0xFFFF)
/* +-80mv, 2.5uV per LSB (fixed) */
/* If averaging is enabled, this is averaged value */
#define REG_SHUNT_UV_PER_LSB	((float)2.50)

/* == REG_BUS definition (unsigned value) == */
#define REG_BUS_MASK		(0x7FFF)
/* 1.25mV per LSB (fixed) */
#define REG_BUS_UV_PER_LSB	((float)1250)

/* == REG_POWER definition (unsigned value) == */
#define REG_POWER_MASK		(0xFFFF)
#define REG_POWER_LSB_RATIO  25

/* == REG_CURRENT definition (signed value) == */
#define REG_CURRENT_MASK	(0xFFFF)

/* == REG_CAL definition (value) == */
#define	REG_CAL_MASK		(0x7FFF)

/*
 * Define a standard current LSB value that is applicable to most
 * scenarios. This value can be made configurable, but establishing a
 * standard baseline ensures most users don't have to manually configure
 * this setting.
 * XXX: NOTE:
 * With this default setting, the maximum measureable current
 * is 16.380 A - Do we really need to go beyond that?? Cough!
 * # .5 mA per LSB (configurable?)
 */
#define REG_CAL_CURRENT_STD_LSB_mA  ((float)0.0005)

#define REG_CAL_VAL_CONST ((float)0.00512)

#define FLIP_BYTES(V)	((((V) & 0xFF00) >> 8) | (((V) & 0x00FF) << 8))

#endif /* __INA226_REG_DEFNS__ */
