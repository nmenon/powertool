/*
 * INA226 common files (sanbox and non-sandbox variants)
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

int __weak ina226_parse_config(struct ina226_rail *rail,
			       char *param, void *data)
{
	if (!strncmp(param, "group", 5)) {
		strncpy(rail->board_group_name, data, INA226_MAX_STRING);
		return 0;
	}

	if (!strncmp(param, "input", 5)) {
		strncpy(rail->in_board_name, data, INA226_MAX_STRING);
		return 0;
	}

	if (!strncmp(param, "output", 5)) {
		strncpy(rail->out_board_name, data, INA226_MAX_STRING);
		return 0;
	}

	if (!strncmp(param, "address", 5)) {
		int ret;
		int val;
		const char *str = (const char *)data;
		ret = sscanf(str, "%x", &val);
		if (!ret) {
			fprintf(stderr, "Parse error of address for %s\n",
				rail->name);
			return -1;
		}
		rail->i2c_slave_addr = val;
		return 0;
	}

	if (!strncmp(param, "shunt_value", 11)) {
		int ret;
		float val;
		const char *str = (const char *)data;
		ret = sscanf(str, "%f", &val);
		if (!ret) {
			fprintf(stderr, "Parse error of shunt_value for %s\n",
				rail->name);
			return -1;
		}
		rail->shunt_resistor_value = val;
		return 0;
	}

	if (!strncmp(param, "shunt_accuracy", 14)) {
		int ret;
		float val;
		const char *str = (const char *)data;
		ret = sscanf(str, "%f", &val);
		if (!ret) {
			fprintf(stderr,
				"Parse error of shunt_accuracy for %s\n",
				rail->name);
			return -1;
		}
		rail->shunt_resistor_accuracy = val;
		return 0;
	}

	/* Umm... i dont know to deal with what you ask me */
	return -1;
}

struct __weak ina226_rail *ina226_find(struct ina226_rail *root_rail,
				       const char *rail_name)
{
	struct ina226_rail *node = root_rail;

	while (node) {
		if (!strncmp(node->name, rail_name, INA226_MAX_STRING))
			return node;
		node = node->next;
	}

	return NULL;
}

struct __weak ina226_rail *ina226_find_or_allocate(struct ina226_rail
						   **root_rail,
						   const char *rail_name)
{

	struct ina226_rail *node;

	node = ina226_find(*root_rail, rail_name);
	if (node)
		return node;

	node = calloc(sizeof(*node), 1);

	/* Put some handy defaults in there */
	node->shunt_conv_time = 4;
	node->bus_conv_time = 4;
	node->shunt_resistor_accuracy = 5;
	strncpy(node->name, rail_name, INA226_MAX_STRING);

	node->next = *root_rail;
	*root_rail = node;

	return node;
}

int __weak ina226_alloc_data_buffers(struct ina226_rail *rail, int num)
{
	if (rail->data)
		free(rail->data);

	rail->data = malloc(sizeof(*(rail->data)) * num);

	if (!rail->data)
		return -ENOMEM;
	return 0;
}

void __weak ina226_free(struct ina226_rail *rail)
{
	if (rail->data)
		free(rail->data);
	free(rail);
}
