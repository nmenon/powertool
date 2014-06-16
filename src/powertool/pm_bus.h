/*
 *  I2C Bus information - this is the root data structure
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
#ifndef __PM_BUS_H_
#define __PM_BUS_H_

#include "ina226.h"

#define MAX_PMBUS_NAME		20
#define MAX_PMBUS_I2CNAME	30

struct pm_bus {
	struct pm_bus *next;
	char name[MAX_PMBUS_NAME];
	char i2c_bus[MAX_PMBUS_I2CNAME];
	struct ina226_rail *rail;
	void *i2c_fd;
};

extern struct pm_bus *root_pm_bus;

struct pm_bus *pm_bus_find(struct pm_bus *root, const char *name);
struct pm_bus *pm_bus_find_or_allocate(struct pm_bus **root, const char *name);
void pm_bus_set_i2c_name(struct pm_bus *node, void *data);
void pm_bus_free(struct pm_bus *node);

#endif	/* __PM_BUS_H_ */
