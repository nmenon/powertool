/*
 * Algorithms
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <version.h>
#include <autoadjust_table.h>

#include "ina226.h"
#include "pm_bus.h"
#include "algo.h"

#define DELIMITER "-----8<-------\n"

void print_algo_list(void)
{
	fprintf(stderr,
		"\nalgo_name options:\n"
		"\tdump: just dumps the data in csv format to stdout\n"
		"\taverage: samples are averaged and final average results"
		"are shown\n");
}

int algo_check(char *algo)
{
	if (!strcmp(algo, "dump") || !strcmp(algo, "average"))
		return 0;
	return 1;
}

static void algo_dump_data(struct pm_bus *bus, int num, int dur_ms)
{
	struct ina226_rail *rail;
	int i;

	fprintf(stdout, DELIMITER "Data Dump Start\n" DELIMITER);
	while (bus) {
		rail = bus->rail;
		while (rail) {
			struct power_data_sample *data = rail->data;

			fprintf(stdout, "%s, %s, %s, %s, %s, %f, %f\n",
				bus->name, rail->name,
				rail->in_board_name,
				rail->out_board_name,
				rail->board_group_name,
				rail->shunt_resistor_value,
				rail->shunt_resistor_accuracy);
			fprintf(stdout,
				"index, shunt_uV, rail_uV, current_mA, power_mW\n");

			for (i = 0; i < num; i++, data++)
				fprintf(stdout, "%d, %f, %f, %f, %f\n",
					i, data->shunt_uV,
					data->rail_uV,
					data->current_mA, data->power_mW);

			fprintf(stdout, DELIMITER);
			rail = rail->next;
		}
		bus = bus->next;
	}

	return;
}

static void algo_average_data(struct pm_bus *bus, int num, int dur_ms)
{
	char table[TABLE_MAX_ROW][TABLE_MAX_COL][TABLE_MAX_ELT_LEN];
	struct ina226_rail *rail;
	int i, ridx;
	int row;
	float current_summary_bus;
	float current_summary_cumulative = 0;

	fprintf(stdout, DELIMITER "Average Data Start\n" DELIMITER);
	while (bus) {
		rail = bus->rail;
		current_summary_bus = 0;
		autoadjust_table_init(table);
		row = 0;
		snprintf(table[row][0], TABLE_MAX_ELT_LEN,
			 "BUS(%s) I2C(%s) AVG(Samples=%d, Interval=%d ms)",
			 bus->name, bus->i2c_bus, num, dur_ms);
		row++;
		strncpy(table[row][0], "Index", TABLE_MAX_ELT_LEN);
		strncpy(table[row][1], "Rail Name", TABLE_MAX_ELT_LEN);
		strncpy(table[row][2], "Shunt voltage(uV)", TABLE_MAX_ELT_LEN);
		strncpy(table[row][3], "Rail voltage(V)", TABLE_MAX_ELT_LEN);
		strncpy(table[row][4], "Current(mA)", TABLE_MAX_ELT_LEN);
		strncpy(table[row][5], "Power(mW)", TABLE_MAX_ELT_LEN);
		row++;
		ridx = 0;

		while (rail) {
			struct power_data_sample *data = rail->data;
			struct power_data_sample avg_sample = { 0 };

			for (i = 0; i < num; i++, data++) {
				avg_sample.shunt_uV =
				    ((avg_sample.shunt_uV * i) +
				     data->shunt_uV) / (i + 1);
				avg_sample.rail_uV =
				    ((avg_sample.rail_uV * i) +
				     data->rail_uV) / (i + 1);
				avg_sample.current_mA =
				    ((avg_sample.current_mA * i) +
				     data->current_mA) / (i + 1);
				avg_sample.power_mW =
				    ((avg_sample.power_mW * i) +
				     data->power_mW) / (i + 1);
			}
			snprintf(table[row][0], TABLE_MAX_ELT_LEN, "%d", ridx);
			snprintf(table[row][1], TABLE_MAX_ELT_LEN, "%s",
				 rail->name);
			snprintf(table[row][2], TABLE_MAX_ELT_LEN, "%3.2f",
				 avg_sample.shunt_uV);
			snprintf(table[row][3], TABLE_MAX_ELT_LEN, "%3.6f",
				 avg_sample.rail_uV / 1000000);
			snprintf(table[row][4], TABLE_MAX_ELT_LEN, "%3.2f",
				 avg_sample.current_mA);
			snprintf(table[row][5], TABLE_MAX_ELT_LEN, "%3.2f",
				 avg_sample.power_mW);
			row++;

			current_summary_bus += avg_sample.power_mW;
			rail = rail->next;
			ridx++;
		}

		snprintf(table[row][0], TABLE_MAX_ELT_LEN, "---");
		snprintf(table[row][1], TABLE_MAX_ELT_LEN, "---");
		snprintf(table[row][2], TABLE_MAX_ELT_LEN, "---");
		snprintf(table[row][3], TABLE_MAX_ELT_LEN, "---");
		snprintf(table[row][4], TABLE_MAX_ELT_LEN, "---");
		snprintf(table[row][5], TABLE_MAX_ELT_LEN, "---");
		row++;
		snprintf(table[row][0], TABLE_MAX_ELT_LEN, "Total");
		snprintf(table[row][5], TABLE_MAX_ELT_LEN, "%3.2f",
			 current_summary_bus);
		row++;
		current_summary_cumulative += current_summary_bus;

		bus = bus->next;
		autoadjust_table_generic_fprint(stdout, table, row, 6,
						TABLE_HAS_SUBTITLE |
						TABLE_HAS_TITLE);
	}
	autoadjust_table_init(table);
	row = 0;
	snprintf(table[row][0], TABLE_MAX_ELT_LEN, "Cumulative Current:");
	snprintf(table[row][1], TABLE_MAX_ELT_LEN,
		 "%3.2f mW", current_summary_cumulative);
	row++;
	autoadjust_table_generic_fprint(stdout, table, row, 2, 0);

	fprintf(stdout, DELIMITER);
	return;
}

void algo_process_data(char *algo, int num_samples, int sampling_duration_ms)
{
	struct pm_bus *bus = root_pm_bus;

	if (!strcmp(algo, "dump"))
		algo_dump_data(bus, num_samples, sampling_duration_ms);
	if (!strcmp(algo, "average"))
		algo_average_data(bus, num_samples, sampling_duration_ms);
}
