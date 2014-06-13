/*
 * Definition of what a power data looks like
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
#ifndef __POWER_DATA_H_
#define __POWER_DATA_H_

#include <proj-types.h>

/**
 * struct power_data_sample - Represents 1 sample of data collected
 * @shunt_uV:	voltage in micro-volts measured access the Shunt resistor
 * @rail_uV:	voltage in micro-volts measured at the voltage rail
 * @current_mA:	Current in mA measured at the rail
 * @power_mW:	Power consumption in mW for the rail
 */
struct power_data_sample {
	float shunt_uV;
	float rail_uV;
	float current_mA;
	float power_mW;
};

#endif	/* __POWER_DATA_H_ */
