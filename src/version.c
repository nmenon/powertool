/*
 * Version information standardization
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

#include <stdio.h>
#include <version.h>

void print_version(FILE *stream, char *application_name)
{
	fprintf(stream, "%s: Rev: %s, i2c_tools version: %s, Build Date: %s\n",
		application_name, powertool_version, lib_i2c_revision,
		powertool_builddate);
}
