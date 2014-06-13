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
#ifndef __INA226__
#define __INA226__

#include <proj-types.h>
#include "power_data.h"

#define INA226_MAX_STRING	30

/* Just the registers of the chip */
struct reg_ina226 {
	u16 config;
	u16 shunt;
	u16 bus;
	u16 power;
	u16 current;
	u16 cal;
	u16 irq_enable;
	u16 die_id;
};

/**
 * struct ina226_rail - rail description that this INA is measuring
 * @next:	pointer to the next INA on the i2c bus (NULL if terminated)
 * @i2c_fd:	I2C File descriptor pointing to the right I2C bus
 * @measurement_active: false if no measurement needs to be done (or initalized)
 * @i2c_slave_addr:	i2c slave address
 * @num_avgs:		Num averages (index from config_mode_average_numer)
 * @shunt_conv_time:	index into config_mode_conversion_time
 * @bus_conv_time:	index into config_mode_conversion_time
 * @shunt_resistor_value: Shunt resistor value in OHms
 * @shunt_resistor_accuracy: Shunt resistor accuracy in %
 * @name:		name of the voltage rail (identifier)
 * @in_board_name:	input voltage rail name
 * @out_board_name:	output voltage rail name
 * @board_group_name:	Grouping name
 */
struct ina226_rail {
	struct	ina226_rail *next;

	int	i2c_fd;

	u8	i2c_slave_addr;

	u8	num_avgs;
	u8	shunt_conv_time;
	u8	bus_conv_time;

	float	shunt_resistor_value;
	float	shunt_resistor_accuracy;

	char	name[INA226_MAX_STRING];
	char	in_board_name[INA226_MAX_STRING];
	char	out_board_name[INA226_MAX_STRING];
	char	board_group_name[INA226_MAX_STRING];

	/* internal */
	float	current_lsb_mA; /* configurability only if > 16.380A */
	float	power_lsb;
	u8	op_mode;
	u16	die_id; /* unique identifier per INA chip */
	struct reg_ina226 reg;
	struct power_data_sample *data;
};

int ina226_init(struct ina226_rail *rail);
int ina226_detect(struct ina226_rail *rail);

int ina226_configure(struct ina226_rail *rail);
int ina226_sample_one(struct ina226_rail *rail);
int ina226_process_one(struct ina226_rail *rail, struct power_data_sample *data);

int ina226_parse_config(struct ina226_rail *rail,
				char *param,
				void *data);

struct ina226_rail *ina226_find(
		struct ina226_rail *root_rail,
		const char *rail_name);
struct ina226_rail *ina226_find_or_allocate(
		struct ina226_rail **root_rail,
		const char *rail_name);
int ina226_alloc_data_buffers(struct ina226_rail *rail, int num);
void ina226_free(struct ina226_rail *rail);

struct pm_bus;
int ina226_bus_init_i2c(struct pm_bus *bus);
void ina226_bus_deinit_i2c(struct pm_bus *node);
int ina226_bus_setup(struct ina226_rail *rail);
#endif	/* __INA226__ */
