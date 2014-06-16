/*
 * I2C wrapper for i2c-dev Vs mpsse
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
#ifndef __I2C_WRAP_H_
#define __I2C_WRAP_H_

#include <ctype.h>
#include <proj-types.h>

int i2c_bus_init(char *desc, void **fd);
void i2c_bus_deinit(void *fd);

int i2c_set_slave(void *fd, u8 sa, int force);

#define I2C_REG_SIZE_8		(1 << 0)
#define I2C_REG_SIZE_16		(1 << 1)
#define I2C_REG_SIZE_32		(1 << 2)
#define I2C_DATA_SIZE_8		(1 << 3)
#define I2C_DATA_SIZE_16	(1 << 4)
#define I2C_DATA_SIZE_32	(1 << 5)

int i2c_read(void *fd, u8 flags, u32 ra, u32 * data);
int i2c_write(void *fd, u8 flags, u32 ra, u32 data);

#endif /* __I2C_WRAP_H_ */
