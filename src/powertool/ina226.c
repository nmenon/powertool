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
#include <i2c-wrap.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <version.h>

#include "pm_bus.h"
#include "ina226.h"
#include "ina226_reg_defns.h"
#include "power_data.h"

static int __maybe_unused _ina226_read(void *i2c_fd, u8 reg, u16 * data)
{
	int res = 0;
	u32 d = 0;

	res = i2c_read(i2c_fd, I2C_REG_SIZE_8 | I2C_DATA_SIZE_16, reg,
		       &d);
	if (res < 0)
		return res;
	*data = (u16) d;
	*data = FLIP_BYTES(*data);
	return 0;
}

static int __maybe_unused _ina226_write(void *i2c_fd, u8 reg, u16 data)
{
	int res;
	u32 d = FLIP_BYTES(data);

	res = i2c_write(i2c_fd, I2C_REG_SIZE_8 | I2C_DATA_SIZE_16, reg,
			d);
	return res;
}

static __maybe_unused int _ina226_rmw(void *i2c_fd, u8 reg, u16 mask, u16 clr,
				      u16 set)
{
	int r;
	u16 data = 0;

	r = _ina226_read(i2c_fd, reg, &data);
	if (r)
		return r;
	data &= ~(clr & mask);
	data |= (set & mask);

	return _ina226_write(i2c_fd, reg, data);
}

static void __maybe_unused _ina226_dump_regs(struct ina226_rail *rail,
					     char *dbg_str)
{
	struct reg_ina226 reg = { 0 };

	_ina226_read(rail->i2c_fd, REG_CONFIG, &reg.config);
	_ina226_read(rail->i2c_fd, REG_SHUNT, &reg.shunt);
	_ina226_read(rail->i2c_fd, REG_BUS, &reg.bus);
	_ina226_read(rail->i2c_fd, REG_POWER, &reg.power);
	_ina226_read(rail->i2c_fd, REG_CURRENT, &reg.current);
	_ina226_read(rail->i2c_fd, REG_CAL, &reg.cal);
	_ina226_read(rail->i2c_fd, REG_IRQ_ENABLE, &reg.irq_enable);
	_ina226_read(rail->i2c_fd, REG_DIEID, &reg.die_id);

	printf("--- %s [0x%02x] ---\n"
	       "CONFIG=0x%04x\n"
	       "SHUNT=0x%04x\n"
	       "BUS=0x%04x\n"
	       "POWER=0x%04x\n"
	       "CURRENT=0x%04x\n"
	       "CAL=0x%04x\n"
	       "IRQ_ENABLE=0x%04x\n"
	       "DIE_ID=0x%04x\n",
	       dbg_str, rail->i2c_slave_addr,
	       reg.config,
	       reg.shunt,
	       reg.bus,
	       reg.power, reg.current, reg.cal, reg.irq_enable, reg.die_id);
}

int ina226_init(struct ina226_rail *rail)
{
	u16 data = REG_CONFIG_RESET;
	u16 timeout = 10000;
	struct reg_ina226 *reg = &rail->reg;
	int r;
	float cal_val;

	/* ONLY DO reset here - no other reg access please */
	r = _ina226_write(rail->i2c_fd, REG_CONFIG, data);
	if (r)
		return r;

	while (data & REG_CONFIG_RESET) {
		/* Usually exits in a single iteration */
		r = _ina226_read(rail->i2c_fd, REG_CONFIG, &data);
		if (r)
			return r;
		timeout--;
		if (!timeout)
			return -ETIMEDOUT;
	}

	/* All heavy computation stuff here please */
	rail->current_lsb_mA = REG_CAL_CURRENT_STD_LSB_mA;

	rail->power_lsb = rail->current_lsb_mA * REG_POWER_LSB_RATIO;
	rail->op_mode = CONFIG_MODE_CONTINOUS(CONFIG_MODE_V_BUS_VOLT |
					      CONFIG_MODE_V_SHUNT_VOLT);

	reg->config = SAFE_SET(rail->num_avgs, REG_CONFIG_AVG_MASK);
	reg->config |= SAFE_SET(rail->shunt_conv_time, REG_SHUNT_MASK);
	reg->config |= SAFE_SET(rail->bus_conv_time, REG_BUS_MASK);
	reg->config |= SAFE_SET(rail->op_mode, REG_CONFIG_MODE_MASK);

	/*
	 * Calculate the INA calibration value
	 * Formula:
	 *     0.00512 (fixed constant)
	 * --------------------------------
	 * self.current_lsb * self.shunt_r
	 */
	cal_val = REG_CAL_VAL_CONST;
	cal_val /= rail->current_lsb_mA * (rail->shunt_resistor_value /
					   (float)1000);
	/*
	 * ???? should add a out of range check? truncation seems to work?
	 * Original python code was truncation logic
	 *      if (cal_val > (float)REG_CAL_MASK)
	 *              reg->cal = REG_CAL_MASK;
	 *      else
	 */
	reg->cal = (u16) cal_val;

	return 0;
}

int ina226_detect(struct ina226_rail *rail)
{
	/* Am I there? I should have a DIEID register.. */
	return _ina226_read(rail->i2c_fd, REG_DIEID, &rail->die_id);
}

int ina226_configure(struct ina226_rail *rail)
{
	int r;
	struct reg_ina226 *reg = &rail->reg;

	r = _ina226_write(rail->i2c_fd, REG_CONFIG, reg->config);
	if (r)
		return r;
	r = _ina226_write(rail->i2c_fd, REG_CAL, reg->cal);
	if (r)
		return r;

	return 0;
}

int ina226_sample_one(struct ina226_rail *rail)
{
	int r;
	struct reg_ina226 *reg = &rail->reg;

	r = _ina226_read(rail->i2c_fd, REG_SHUNT, &reg->shunt);
	if (r)
		return r;

	r = _ina226_read(rail->i2c_fd, REG_BUS, &reg->bus);
	if (r)
		return r;

	r = _ina226_read(rail->i2c_fd, REG_POWER, &reg->power);
	if (r)
		return r;

	r = _ina226_read(rail->i2c_fd, REG_CURRENT, &reg->current);

	return r;
}

static s16 convert_to_decimal(u16 x)
{
	u16 m = x >> 16;
	return (~m & x) | (((x & 0x8000) - x) & m);
}

int ina226_process_one(struct ina226_rail *rail, struct power_data_sample *data)
{
	struct reg_ina226 *reg = &rail->reg;

	data->shunt_uV =
	    ((float)convert_to_decimal(reg->shunt & REG_SHUNT_MASK)) *
	    REG_SHUNT_UV_PER_LSB;

	data->rail_uV = ((float)(reg->bus & REG_BUS_MASK)) * REG_BUS_UV_PER_LSB;

	data->current_mA =
	    ((float)convert_to_decimal(reg->current & REG_CURRENT_MASK)) *
	    rail->current_lsb_mA * 1000;

	data->power_mW =
	    ((float)(reg->power & REG_POWER_MASK)) * rail->power_lsb * 1000;

	return 0;
}

int ina226_bus_init_i2c(struct pm_bus *bus)
{
	int ret;
	void *fd;
	struct ina226_rail *rail = bus->rail;

	ret = i2c_bus_init(bus->i2c_bus, &fd);
	if (ret < 0)
		return ret;

	bus->i2c_fd = fd;

	if (!rail)
		return 0;

	while (rail) {
		rail->i2c_fd = bus->i2c_fd;
		rail = rail->next;
	}

	return 0;
}

void ina226_bus_deinit_i2c(struct pm_bus *bus)
{
	i2c_bus_deinit(bus->i2c_fd);
}

int ina226_bus_setup(struct ina226_rail *rail)
{
	return i2c_set_slave(rail->i2c_fd, rail->i2c_slave_addr, 1);
}
