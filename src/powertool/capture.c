/*
 * Logic that controls the capture of data
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
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <version.h>

#include "ina226.h"
#include "pm_bus.h"
#include "capture.h"

static inline int _init_allbuses(struct pm_bus *bus)
{
	int ret = 0;
	struct pm_bus *start_bus = bus;
	struct ina226_rail *rail;

	/* Ensure that we have file handle for all buses */
	while (bus) {
		ret = ina226_bus_init_i2c(bus);
		if (ret) {
			fprintf(stderr, "Setup of bus %s failed %d\n",
				bus->name, ret);
			goto out;
		}
		bus = bus->next;
	}

	bus = start_bus;
	while (bus) {
		rail = bus->rail;

		while (rail) {
			ret = ina226_bus_setup(rail);
			if (ret) {
				fprintf(stderr, "setup %s:%s fail:%d\n",
					bus->name, rail->name, ret);
				goto out;
			}

			ret = ina226_detect(rail);
			if (ret) {
				fprintf(stderr, "detect %s:%s fail:%d\n",
					bus->name, rail->name, ret);
				goto out;
			}

			ret = ina226_init(rail);
			if (ret) {
				fprintf(stderr, "init %s:%s fail:%d\n",
					bus->name, rail->name, ret);
				goto out;
			}

			rail = rail->next;
		}

		bus = bus->next;
	}

out:
	return ret;
}

static inline void _deinit_allbuses(struct pm_bus *bus)
{
	while (bus) {
		/* Rails dont need to be de-inited */
		ina226_bus_deinit_i2c(bus);
		bus = bus->next;
	}
}

static int _setup_allbuses(struct pm_bus *bus, int num)
{
	int ret;
	struct ina226_rail *rail;

	while (bus) {
		rail = bus->rail;

		while (rail) {
			/* buffers are freed after final data processing */
			ret = ina226_alloc_data_buffers(rail, num);
			if (ret) {
				fprintf(stderr, "alloc %s:%s fail:%d\n",
					bus->name, rail->name, ret);
				goto out;
			}

			ret = ina226_bus_setup(rail);
			if (ret) {
				fprintf(stderr, "setup failed %s:%s:%d\n",
					bus->name, rail->name, ret);
				goto out;
			}

			ret = ina226_configure(rail);
			if (ret) {
				fprintf(stderr, "configure failed %s:%s:%d\n",
					bus->name, rail->name, ret);
				goto out;
			}
			rail = rail->next;
		}

		bus = bus->next;
	}
out:
	return ret;
}

static int _capture_data_allbuses(struct pm_bus *bus, int idx)
{
	int ret;
	struct ina226_rail *rail;

	while (bus) {
		rail = bus->rail;

		while (rail) {
			ret = ina226_bus_setup(rail);
			if (ret) {
				fprintf(stderr, "[%d]setup failed %s:%s:%d\n",
					idx, bus->name, rail->name, ret);
				goto out;
			}

			ret = ina226_sample_one(rail);
			if (ret) {
				fprintf(stderr,
					"[%d]sample_one failed %s:%s:%d\n", idx,
					bus->name, rail->name, ret);
				goto out;
			}
			rail = rail->next;
		}

		bus = bus->next;
	}
out:
	return ret;
}

static int _process_data_allbuses(struct pm_bus *bus, int idx)
{
	int ret;
	struct ina226_rail *rail;

	while (bus) {
		rail = bus->rail;

		while (rail) {
			/* No bus setup is needed for data processing */
			ret = ina226_process_one(rail, &(rail->data[idx]));
			if (ret) {
				fprintf(stderr,
					"[%d]process_one failed %s:%s:%d\n",
					idx, bus->name, rail->name, ret);
				goto out;
			}
			rail = rail->next;
		}

		bus = bus->next;
	}
out:
	return ret;
}

int capture_data(int num, int dur_ms)
{
	struct pm_bus *bus = root_pm_bus;
	int ret;
	int i = 0;
	float seconds, delta;
	long d_us, d_s;
	struct timeval before, after;

	fprintf(stderr, "Initializing bus ...\n");
	ret = _init_allbuses(bus);
	if (ret)
		return ret;

	fprintf(stderr, "Setting up bus and rails...\n");
	ret = _setup_allbuses(bus, num);
	if (ret)
		goto out;

	seconds = (float)num *(float)dur_ms / 1000;

	fprintf(stderr,
		"Capturing data from rails: Approximately %f Seconds..\n",
		seconds);

	gettimeofday(&before, NULL);

	for (i = 0; i < num; i++) {
		ret = _capture_data_allbuses(bus, i);
		if (ret)
			break;

		ret = _process_data_allbuses(bus, i);
		if (ret)
			break;
		msleep(dur_ms);
	}

	gettimeofday(&after, NULL);

	d_s = after.tv_sec - before.tv_sec;
	d_us = after.tv_usec - before.tv_usec;
	delta = ((float)d_s * 1000) + ((float)d_us / 1000) + 0.5;
	delta /= 1000;
	fprintf(stderr, "Data capture completed. %f Seconds elapsed\n", delta);

out:
	_deinit_allbuses(bus);

	return ret;
}
