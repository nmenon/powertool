/*
 * Main powertool file
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
#include "parse_config.h"
#include "capture.h"
#include "algo.h"

enum powertool_args {
	PWR_VERSION,
	PWR_DISPLAY,
	PWR_BRD_CONFIG,
	PWR_SAMPLING_DUR,
	PWR_NUM_SAMPLES,
	PWR_RAIL,
	PWR_GROUP,
	PWR_ALGO,
};

static const struct option long_options[] = {
	{"version", no_argument, NULL, PWR_VERSION},
	{"display_rails", no_argument, NULL, PWR_DISPLAY},
	{"config_file", required_argument, NULL, PWR_BRD_CONFIG},
	{"sampling_duration", required_argument, NULL, PWR_SAMPLING_DUR},
	{"num_samples", required_argument, NULL, PWR_NUM_SAMPLES},
	{"rail_capture", required_argument, NULL, PWR_RAIL},
	{"group_capture", required_argument, NULL, PWR_GROUP},
	{"algo", required_argument, NULL, PWR_ALGO},
	{0,},
};

#define OPTION_STRING	"dvc:s:n:r:g:a:"

static int usage(char *app_name)
{
	print_version(stderr, app_name);
	fprintf(stderr, "Options:\n"
		"\t-v | --version | --v : prints the version\n"
		"\t-d | --display_rails | --d : Parses the config_file and displays information\n"
		"\t[-c | --config_file | --c] config_file : Loads up the board config file\n"
		"\t[-s | --sampling_duration | --s] sampling_duration : Setsup delay(in ms) between each sample capture (min guarenteed) (default 200ms)\n"
		"\t[-n | --num_samples | --n] num_samples : Number of Samples to capture (default 100)\n"
		"\t[-r | --rail_capture | --r] rail_name : Adds to Rail to capture(max 20)\n"
		"\t[-g | --group_capture | --g] group_name : Adds all rail in the group to capture(max 10)\n"
		"\t[-a | --algo | --a] algo_name : data processing algo to use\n");
	print_algo_list();
	fprintf(stderr, "\nExample:\n"
		"%s --config brd.config --s 100 --n  200 -r mpu -r gpu -g ddr\n"
		"\tUses:\n"
		"\t\tbrd.cfg as configuration file\n"
		"\t\tmpu and gpu are added to the capture list(must be defined in brd.cfg)\n"
		"\t\tAll rails with group=ddr are added to the capture list(must be defined in brd.cfg) - say ddr_3v3 and ddr_cpu\n"
		"\t\tFinal Result:\n"
		"\t\t mpu, gpu, ddr_3v3 and ddr_cpu are sampled every 100ms for 200 samples\n"
		"\t\t Final average of all 200 samples are presented per rail\n"
		"\n----\n"
		"Example: list all rails\n"
		"%s --config brd.config -d\n", app_name, app_name);

	return -EINVAL;
}

/**
 * main() - Main function
 * @argc:	argument count
 * @argv:	arguments
 */
int main(int argc, char *argv[])
{
	int sampling_duration_ms = 200;
	int num_samples = 100;
	char algo[30] = "average";
	char config_file[30];
	char *voltage_rails_to_measure[20];
	char *groups_to_measure[10];
	bool display = false;
	int ret;

	int max_voltage_rails_measure = ARRAY_SIZE(voltage_rails_to_measure);
	int max_groups_measure = ARRAY_SIZE(groups_to_measure);
	int idx_v_rail = 0;
	int idx_group = 0;

	while (1) {
		int option_index = 0;
		int c = getopt_long(argc, argv, OPTION_STRING, long_options,
				    &option_index);
		if (c == -1)
			break;

		switch (c) {
		case 'v':
		case PWR_VERSION:
			print_version(stdout, argv[0]);
			return 0;

		case 'd':
		case PWR_DISPLAY:
			display = true;
			break;

		case 'c':
		case PWR_BRD_CONFIG:
			strncpy(config_file, optarg, sizeof(config_file));
			break;

		case 's':
		case PWR_SAMPLING_DUR:
			sscanf(optarg, "%d", &sampling_duration_ms);
			break;

		case 'n':
		case PWR_NUM_SAMPLES:
			sscanf(optarg, "%d", &num_samples);
			break;

		case 'r':
		case PWR_RAIL:
			if (idx_v_rail >= max_voltage_rails_measure)
				return usage(argv[0]);

			voltage_rails_to_measure[idx_v_rail] = optarg;
			idx_v_rail++;
			break;

		case 'g':
		case PWR_GROUP:
			if (idx_group >= max_groups_measure)
				return usage(argv[0]);

			groups_to_measure[idx_group] = optarg;
			idx_group++;
			break;

		case 'a':
		case PWR_ALGO:
			strncpy(algo, optarg, sizeof(algo));
			break;
		case '?':
		default:
			return usage(argv[0]);
		}
	}

	/* Generic parameter checks */
	if (1 == strlen(config_file)) {
		fprintf(stderr, "Error: need config file\n");
		return usage(argv[0]);
	}

	if (!display) {
		if (!idx_v_rail && !idx_group) {
			fprintf(stderr,
				"Error: Need group of rails to measure\n");
			return usage(argv[0]);
		}

		if (!num_samples || !sampling_duration_ms) {
			fprintf(stderr, "Error: 0 capture attempt?\n");
			return usage(argv[0]);
		}

		if (algo_check(algo)) {
			fprintf(stderr, "Bad Algo %s used\n", algo);
			return usage(argv[0]);
		}

	}

	ret = parse_file(config_file, 5);
	if (ret) {
		fprintf(stderr, "Parse of config file %s failed\n",
			config_file);
		return usage(argv[0]);
	}

	if (display) {
		parse_print_rails();
	} else {
		/* Get rid of un-needed rails */
		parse_cleanup(voltage_rails_to_measure, idx_v_rail,
			      groups_to_measure, idx_group);

		fprintf(stderr, "Voltage Rails selected for Capture:\n");
		parse_print_rails();

		ret = capture_data(num_samples, sampling_duration_ms);
		if (ret) {
			fprintf(stderr, "Data Capture Failed!\n");
			goto out;
		}

		algo_process_data(algo, num_samples, sampling_duration_ms);
	}

out:
	/* Cleanup everything remaining */
	parse_cleanup(NULL, 0, NULL, 0);
	return ret;
}
