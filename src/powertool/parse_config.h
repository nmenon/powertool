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
#ifndef __PARSE_CONFIG_H
#define __PARSE_CONFIG_H

int parse_file(char *fname, int num_samples);

void parse_cleanup(char **rails_to_measure, int num_rails,
		   char **groups_to_measure, int num_groups);

void parse_print_rails(void);
#endif	/* __PARSE_CONFIG_H */
