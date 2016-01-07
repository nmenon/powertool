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
#ifndef __PWR_ALGO__
#define __PWR_ALGO__

int algo_check(char *algo);
void algo_process_data(char *algo, int num_samples, int sampling_duration_ms);
void algo_stream_data(struct pm_bus *bus, int idx);
void algo_stream_data_start(struct pm_bus *bus);
void print_algo_list(void);

#endif	/* __PWR_ALGO__ */
