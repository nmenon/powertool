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

#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <version.h>
#include <proj-types.h>

#include "pm_bus.h"

struct pm_bus *root_pm_bus;

struct pm_bus *pm_bus_find(struct pm_bus *root, const char *name)
{
	struct pm_bus *node = root;
	while (node) {
		if (!strncmp(name, node->name, MAX_PMBUS_NAME))
			return node;
		node = node->next;
	}
	return NULL;
}

struct pm_bus *pm_bus_find_or_allocate(struct pm_bus **root, const char *name)
{
	struct pm_bus *node;

	node = pm_bus_find(*root, name);
	/* If already allocated, just return it */
	if (node)
		return node;

	node = calloc(sizeof(*node), 1);
	if (!node)
		return NULL;

	strncpy(node->name, name, MAX_PMBUS_NAME);

	node->next = *root;
	*root = node;

	return node;
}

void pm_bus_set_i2c_name(struct pm_bus *node, void *data)
{
	/*
	 * We could do some error check to ensure it was not duplicated..
	 * but letting it be to allow overrides
	 */
	strncpy(node->i2c_bus, (char *)data, MAX_PMBUS_I2CNAME);
}

void pm_bus_free(struct pm_bus *node)
{
	free(node);
}
