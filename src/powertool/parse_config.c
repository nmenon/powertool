/*
 * Configuration Parsing file
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
#include <autoadjust_table.h>

#include "lcfg_static.h"
#include "pm_bus.h"
#include "ina226.h"
#include "parse_config.h"

static char **split_pmbus_key(const char *key, int *num_keys)
{
	char **res = NULL;
	char *p = strtok((char *)key, ".");
	int n_keys = 0;
	/* split string and append tokens to 'res' */

	while (p) {
		res = realloc(res, sizeof(char *) * ++n_keys);

		if (res == NULL)
			return NULL;

		res[n_keys - 1] = p;

		p = strtok(NULL, ".");
	}

	/* realloc one extra element for the last NULL */

	res = realloc(res, sizeof(char *) * (n_keys + 1));
	res[n_keys] = NULL;

	*num_keys = n_keys;
	return res;
}

static enum lcfg_status compare_eventhandler(const char *key, void *data,
					     size_t len, void *user_data)
{
	char **key_split = NULL;
	int n_key = 0;
	enum lcfg_status ret = lcfg_status_ok;
	struct pm_bus *bus;
	struct ina226_rail *rail;

	key_split = split_pmbus_key(key, &n_key);
	if (!key_split)
		return lcfg_status_error;

	if (n_key > 3 || n_key == 1) {
		fprintf(stderr, "Invalid entry %s\n", key);
		ret = lcfg_status_error;
		goto out;
	}

	/*
	 * Case 1: i2c bus info
	 * key[0] = bus, key[1] = i2c_bus_name, key[2] = NULL
	 * Case 2: ina rail info
	 * key[0] = bus, key[1] = rail_name, key[2] = rail_name param
	 */
	bus = pm_bus_find_or_allocate(&root_pm_bus, key_split[0]);
	if (!bus) {
		fprintf(stderr, "Unable to allocate bus %s\n", key_split[0]);
		ret = lcfg_status_error;
		goto out;
	}

	if (n_key == 2) {
		if (!strncmp("i2c", key_split[1], 3)) {
			pm_bus_set_i2c_name(bus, data);
			goto out;
		}
		fprintf(stderr, "Unknown bus option %s\n", key_split[1]);
		ret = lcfg_status_error;
		goto out;
	}

	rail = ina226_find_or_allocate(&bus->rail, key_split[1]);
	if (!rail) {
		fprintf(stderr, "Unable to allocate rail %s\n", key_split[1]);
		ret = lcfg_status_error;
		goto out;
	}

	if (n_key == 3)
		ina226_parse_config(rail, key_split[2], data);

out:
	free(key_split);
	return ret;
}

void parse_print_rails(void)
{
	char table[TABLE_MAX_ROW][TABLE_MAX_COL][TABLE_MAX_ELT_LEN];
	int row;
	struct pm_bus *bus;
	struct ina226_rail *rail;

	bus = root_pm_bus;
	while (bus) {
		autoadjust_table_init(table);
		row = 0;
		snprintf(table[row][0], TABLE_MAX_ELT_LEN,
			 "BUS_NAME = %s I2C_BUS=%s", bus->name, bus->i2c_bus);
		row++;
		strncpy(table[row][0], "Rail Name", TABLE_MAX_ELT_LEN);
		strncpy(table[row][1], "Group Name", TABLE_MAX_ELT_LEN);
		strncpy(table[row][2], "Input Name", TABLE_MAX_ELT_LEN);
		strncpy(table[row][3], "Output Name", TABLE_MAX_ELT_LEN);
		strncpy(table[row][4], "I2C Address(hex)", TABLE_MAX_ELT_LEN);
		strncpy(table[row][5], "Shunt Resistor Value(Ohm)",
			TABLE_MAX_ELT_LEN);
		strncpy(table[row][6], "Shunt Resistor Accuracy(%)",
			TABLE_MAX_ELT_LEN);
		row++;

		rail = bus->rail;
		while (rail) {
			snprintf(table[row][0], TABLE_MAX_ELT_LEN, "%s",
				 rail->name);
			snprintf(table[row][1], TABLE_MAX_ELT_LEN, "%s",
				 rail->board_group_name);
			snprintf(table[row][2], TABLE_MAX_ELT_LEN, "%s",
				 rail->in_board_name);
			snprintf(table[row][3], TABLE_MAX_ELT_LEN, "%s",
				 rail->out_board_name);
			snprintf(table[row][4], TABLE_MAX_ELT_LEN, "0x%02x",
				 rail->i2c_slave_addr);
			snprintf(table[row][5], TABLE_MAX_ELT_LEN, "%4.3f",
				 rail->shunt_resistor_value);
			snprintf(table[row][6], TABLE_MAX_ELT_LEN, "%3.1f",
				 rail->shunt_resistor_accuracy);
			row++;
			rail = rail->next;
		}
		bus = bus->next;
		autoadjust_table_generic_fprint(stdout, table, row, 7,
						TABLE_HAS_SUBTITLE |
						TABLE_HAS_TITLE);
	}

}

int parse_file(char *fname, int num_samples)
{
	struct lcfg *c = lcfg_new(fname);
	enum lcfg_status stat;
	if (!c) {
		fprintf(stderr, "Config file %s: init error\n", fname);
		return -EINVAL;
	}

	stat = lcfg_parse(c);
	if (stat != lcfg_status_ok) {
		fprintf(stderr, "Config file %s: parse error(%d)\n", fname,
			stat);
		return -EINVAL;
	}

	lcfg_accept(c, compare_eventhandler, NULL);

	lcfg_delete(c);
	return 0;
}

static inline int find_match(char *val, char **list, int num_list)
{
	int i;

	for (i = 0; i < num_list; i++)
		if (!strcasecmp(val, list[i]))
			return 1;

	return 0;
}

int parse_validate(char **rails_to_measure, int num_rails,
		   char **groups_to_measure, int num_groups)
{
	struct pm_bus *bus = root_pm_bus;
	struct ina226_rail *rail;
	char **validated_rails = NULL;
	char **validated_groups = NULL;
	int n_valid_rails = 0, n_valid_groups = 0, ret = 0, i;

	if (num_rails) {
		validated_rails = calloc(sizeof(*validated_rails), num_rails);
		if (!validated_rails) {
			fprintf(stderr,
				"Unable to allocate memory for rails\n");
			ret = ENOMEM;
			goto out;
		}
	}

	if (num_groups) {
		validated_groups = calloc(sizeof(*validated_groups),
					  num_groups);
		if (!validated_groups) {
			fprintf(stderr,
				"Unable to allocate memory for groups\n");
			ret = ENOMEM;
			goto out;
		}
	}

	while (bus) {
		rail = bus->rail;
		while (rail) {
			if (find_match(rail->name, rails_to_measure, num_rails)) {
				if (!find_match(rail->name,
						validated_rails,
						n_valid_rails)) {
					validated_rails[n_valid_rails] =
					    rail->name;
					n_valid_rails++;
				} else {
					fprintf(stderr,
						"Bad arg %s duplicated\n",
						rail->name);
					ret = -EINVAL;
					goto out;
				}
			}

			if (find_match(rail->board_group_name,
				       groups_to_measure, num_groups)) {
				if (!find_match(rail->board_group_name,
						validated_groups,
						n_valid_groups)) {
					validated_groups[n_valid_groups] =
					    rail->board_group_name;
					n_valid_groups++;
				}
			}
			rail = rail->next;
		}
		bus = bus->next;
	}

	/* Verification */
	if (n_valid_rails < num_rails) {
		for (i = 0; i < num_rails; i++) {
			if (!find_match(rails_to_measure[i],
					validated_rails, n_valid_rails))
				fprintf(stderr, "invalid rail %s\n",
					rails_to_measure[i]);
		}
		ret = -EINVAL;
	}

	if (n_valid_groups < num_groups) {
		for (i = 0; i < num_groups; i++) {
			if (!find_match(groups_to_measure[i],
					validated_groups, n_valid_groups))
				fprintf(stderr, "invalid group %s\n",
					groups_to_measure[i]);
		}
		ret = -EINVAL;
	}

out:
	if (ret) {
		fprintf(stderr,
			"You may want to use '-d' option to list rails\n");
	}

	if (validated_rails)
		free(validated_rails);
	if (validated_groups)
		free(validated_groups);

	return ret;
}

void parse_cleanup(char **rails_to_measure, int num_rails,
		   char **groups_to_measure, int num_groups)
{
	struct pm_bus *bus = root_pm_bus, *to_free_bus, *prev_bus = NULL;
	struct ina226_rail *rail, *to_free_rail, *prev_rail = NULL;

	while (bus) {
		rail = bus->rail;
		while (rail) {
			/* is this rail a matching rail? if yes, skip */
			if (find_match(rail->name, rails_to_measure, num_rails)
			    || find_match(rail->board_group_name,
					  groups_to_measure, num_groups)) {
				prev_rail = rail;
				rail = rail->next;
				continue;
			}

			to_free_rail = rail;
			rail = rail->next;
			/* unlink it */
			if (to_free_rail == bus->rail)
				bus->rail = rail;
			else
				prev_rail->next = rail;
			ina226_free(to_free_rail);
		}

		if (bus->rail) {
			prev_bus = bus;
			bus = bus->next;
			continue;
		}

		to_free_bus = bus;
		bus = bus->next;
		/* unlink it */
		if (to_free_bus == root_pm_bus)
			root_pm_bus = bus;
		else
			prev_bus->next = bus;
		pm_bus_free(to_free_bus);
	}
}
