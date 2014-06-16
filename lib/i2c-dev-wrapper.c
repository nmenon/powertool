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
#include <i2cbusses.h>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <version.h>
#include <i2c-wrap.h>

int i2c_bus_init(char *desc, void **f_d)
{
	char fname[20];
	int i2c_bus;
	int fd;

	if (!f_d || !desc)
		return -EINVAL;

	i2c_bus = lookup_i2c_bus(desc);
	if (i2c_bus < 0)
		return i2c_bus;

	fd = open_i2c_dev(i2c_bus, fname, sizeof(fname), 0);
	if (fd < 0)
		return fd;

	*f_d = (void *)fd;

	return 0;
}

void i2c_bus_deinit(void *f_d)
{
	int fd = (int)f_d;
	close(fd);
}

int i2c_set_slave(void *f_d, u8 sa, int force)
{
	int fd = (int)f_d;
	return set_slave_addr(fd, sa, force);
}

int i2c_read(void *f_d, u8 flags, u32 ra, u32 * data)
{
	int fd = (int)f_d;
	int ret = 0;
	u8 d_flag = (flags & (0x7 << 3));
	u8 a_flag = (flags & (0x7 << 0));

	if (a_flag != I2C_REG_SIZE_8)
		return -ENOSYS;

	switch (d_flag) {
	case I2C_DATA_SIZE_8:
		ret = i2c_smbus_read_byte_data(fd, ra);
		if (ret < 0)
			goto out;
		*data = ret;
		ret = 0;
		break;
	case I2C_DATA_SIZE_16:
		ret = i2c_smbus_read_word_data(fd, ra);
		if (ret < 0)
			goto out;
		*data = ret;
		ret = 0;
		break;
	case I2C_DATA_SIZE_32:
		/* No support yet */
	default:
		return -ENOSYS;
	}
out:
	return ret;
}

int i2c_write(void *f_d, u8 flags, u32 ra, u32 data)
{
	int fd = (int)f_d;
	int ret = 0;
	u8 d_flag = (flags & (0x7 << 3));
	u8 a_flag = (flags & (0x7 << 0));

	if (a_flag != I2C_REG_SIZE_8)
		return -ENOSYS;

	switch (d_flag) {
	case I2C_DATA_SIZE_8:
		ret = i2c_smbus_write_byte_data(fd, ra, data);
		if (ret < 0)
			goto out;
		ret = 0;
		break;
	case I2C_DATA_SIZE_16:
		ret = i2c_smbus_write_word_data(fd, ra, data);
		if (ret < 0)
			goto out;
		ret = 0;
		break;
	case I2C_DATA_SIZE_32:
		/* No support yet */
	default:
		return -ENOSYS;
	}
out:
	return ret;
}
