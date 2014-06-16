/*
 * I2C wrapper for i2c-dev
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
#include <i2c-wrap.h>

int i2c_bus_init(char *desc, void **f_d)
{
	*f_d = NULL;
	return 0;
}

void i2c_bus_deinit(void *f_d)
{
}

int i2c_set_slave(void *f_d, u8 sa, int force)
{
	return 0;
}

int i2c_read(void *f_d, u8 flags, u32 ra, u32 * data)
{
	return 0;
}

int i2c_write(void *f_d, u8 flags, u32 ra, u32 data)
{
	return 0;
}
