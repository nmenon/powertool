/*
 * INA226 Usage in power measurement
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

#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <version.h>

#include "ina226.h"
#include "power_data.h"

int __weak ina226_init(struct ina226_rail *rail)
{
	return 0;
}

int __weak ina226_detect(struct ina226_rail *rail)
{
	return 0;
}

int __weak ina226_configure(struct ina226_rail *rail)
{
	return 0;
}

int __weak ina226_sample_one(struct ina226_rail *rail)
{
	return 0;
}

int __weak ina226_process_one(struct ina226_rail *rail,
			      struct power_data_sample *data)
{
	data->shunt_uV = (float)rand();

	data->rail_uV = (float)rand();

	data->current_mA = (float)rand();

	data->power_mW = (float)rand();

	return 0;
}

int __weak ina226_bus_init_i2c(struct pm_bus *bus)
{
	return 0;
}

void __weak ina226_bus_deinit_i2c(struct pm_bus *node)
{
	return;
}

int __weak ina226_bus_setup(struct ina226_rail *rail)
{
	return 0;
}
