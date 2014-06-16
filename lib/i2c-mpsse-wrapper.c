/*
 * I2C wrapper for i2c-dev
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
#include <mpsse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <version.h>
#include <i2c-wrap.h>

struct my_mpsse_wrapper {
	struct mpsse_context *ctx;
	u8 sa_wr;
	u8 sa_rd;

};
#define DUMMY_SERIAL "FTVFSHEZ"
#define DUMMY_ID "0x0403:0x6010:1:"
int i2c_bus_init(char *desc, void **f_d)
{
	struct mpsse_context *ctx;
	struct my_mpsse_wrapper *w;
	char str_desc[30];
	char *iVendor, *iProduct, *Iface, *iSerial;
	char *saveptr;
	int vendor_id, product_id, interface;
	enum interface i_f;

	if (strlen(desc) < strlen(DUMMY_ID))
		goto err_params;

	strncpy(str_desc, desc, sizeof(str_desc));
	iVendor=strtok_r(str_desc, ":", &saveptr);
	iProduct=strtok_r(NULL, ":", &saveptr);
	Iface=strtok_r(NULL, ":", &saveptr);
	iSerial=strtok_r(NULL, ":", &saveptr);
	/* We expect valid values */
	if (!iVendor || !iProduct || !Iface || !iSerial)
		goto err_params;

	sscanf(iVendor,"0x%04x", &vendor_id);
	sscanf(iProduct,"0x%04x", &product_id);
	sscanf(Iface,"%d", &interface);

	switch (interface) {
		case 1:
			i_f = IFACE_A;
			break;
		case 2:
			i_f = IFACE_B;
			break;
		case 3:
			i_f = IFACE_C;
			break;
		case 4:
			i_f = IFACE_D;
			break;
		default:
			fprintf(stderr, "valid interface 1-4\n");
			goto err_params;
	}


	ctx = Open(vendor_id, product_id, I2C, ONE_HUNDRED_KHZ, MSB,
		   i_f, NULL, iSerial);

	if (!ctx) {
		fprintf(stderr, "Uh uh.. failed to open\n");
		return -EINVAL;
	}
	if (!ctx->open) {
		fprintf(stderr, "Uh uh.. Opening the device Fail!\n");
		Close(ctx);
		return -EINVAL;
	}

	w = calloc(sizeof(*w), 1);
	if (!w) {
		Close(ctx);
		return -ENOMEM;
	}
	w->ctx = ctx;

	*f_d = (void *)w;

	return 0;

err_params:
	fprintf(stderr, "MPSSE I2C ID should look like: "
		"vendorid:productid:interface:iSerial. not \"%s\"\n", desc);
	fprintf(stderr, "Example: \"%s%s\". "
		"Check configuration file\n",
		DUMMY_ID, DUMMY_SERIAL);
	return -EINVAL;
}

void i2c_bus_deinit(void *f_d)
{
	struct my_mpsse_wrapper *w = (struct my_mpsse_wrapper *)f_d;

	Close(w->ctx);
	free(w);
}

int i2c_set_slave(void *f_d, u8 sa, int force)
{
	struct my_mpsse_wrapper *w = (struct my_mpsse_wrapper *)f_d;

	if (!w)
		return -EINVAL;

	/* Actual 8 bit is a 7 bit + 1 bit to indicate rd/wr */
	sa <<= 1;
	w->sa_wr = sa | 0x0;
	w->sa_rd = sa | 0x1;

	return 0;
}

static inline int i2c_send_onebyte(struct mpsse_context *ctx, char *op, u8 d)
{
	int ret;
	char data = (char)d;

	ret = Write(ctx, &data, 1);
	if (ret) {
		fprintf(stderr, "Failed to send %s: %d\n", op, ret);
		return ret;
	}

	ret = GetAck(ctx);
	if (ret != ACK) {
		fprintf(stderr, "Failed to get an ACK for %s: %d\n", op, ret);
		return -EINVAL;
	}
	return ret;
}

static __maybe_unused inline int i2c_read_onebyte(struct mpsse_context *ctx,
						  char *op, u8 * d)
{
	int ret = 0;
	char *data;

	data = Read(ctx, 1);
	if (!data) {
		ret = -EINVAL;
		fprintf(stderr, "Failed to Read %s: %d\n", op, ret);
	} else {
		*d = data[0];
	}
	free(data);
	return ret;
}

static inline int i2c_send_slaveaddr(struct mpsse_context *ctx, u8 sa)
{
	int ret;

	/* Start condition */
	ret = Start(ctx);
	if (ret) {
		fprintf(stderr, "Failed to send start condition: %d\n", ret);
		return ret;
	}
	ret = i2c_send_onebyte(ctx, "SA", sa);

	if (ret)
		Stop(ctx);

	return ret;
}

int i2c_read(void *f_d, u8 flags, u32 ra, u32 * data)
{
	struct my_mpsse_wrapper *w = (struct my_mpsse_wrapper *)f_d;
	struct mpsse_context *ctx;
	u8 d_flag = (flags & (0x7 << 3));
	u8 a_flag = (flags & (0x7 << 0));
	char *rx_buf = NULL;
	u8 len = 0;
	int ret = 0;
	int i;

	if (a_flag != I2C_REG_SIZE_8)
		return -ENOSYS;

	if (!w)
		return -EINVAL;
	ctx = w->ctx;

	ret = i2c_send_slaveaddr(ctx, w->sa_wr);
	if (ret)
		return ret;

	ret = i2c_send_onebyte(ctx, "Register address", (u8) ra);
	if (ret)
		goto out;

	/* Now, the read op */
	ret = i2c_send_slaveaddr(ctx, w->sa_rd);
	if (ret)
		goto out;

	switch (d_flag) {
	case I2C_DATA_SIZE_8:
		len = 1;
		break;
	case I2C_DATA_SIZE_16:
		len = 2;
		break;
	case I2C_DATA_SIZE_32:
		/* No support yet */
	default:
		return -ENOSYS;
	}

	rx_buf = Read(ctx, len);
	if (!rx_buf) {
		fprintf(stderr, "Failed read data[%d]:\n", len);
		ret = -EINVAL;
		goto out;
	}

	for (i = 0; i < len; i++)
		*data |= rx_buf[i] << (8 * i);

	free(rx_buf);

out:
	Stop(ctx);

	return ret;
}

int i2c_write(void *f_d, u8 flags, u32 ra, u32 data)
{
	struct my_mpsse_wrapper *w = (struct my_mpsse_wrapper *)f_d;
	struct mpsse_context *ctx;
	u8 d_flag = (flags & (0x7 << 3));
	u8 a_flag = (flags & (0x7 << 0));
	u8 send_data[4] = { 0 };
	u8 len = 0;
	int ret = 0;
	int i = 0;

	if (a_flag != I2C_REG_SIZE_8)
		return -ENOSYS;

	if (!w)
		return -EINVAL;
	ctx = w->ctx;

	send_data[len] = ra & 0xFF;
	len++;

	switch (d_flag) {
	case I2C_DATA_SIZE_8:
		send_data[len] = data & 0xFF;
		len++;
		break;
	case I2C_DATA_SIZE_16:
		send_data[len] = data & 0xFF;
		len++;
		send_data[len] = (data >> 8) & 0xFF;
		len++;
		break;
	case I2C_DATA_SIZE_32:
		/* No support yet */
	default:
		return -ENOSYS;
	}

	ret = i2c_send_slaveaddr(ctx, w->sa_wr);
	if (ret)
		return ret;

	for (i = 0; i < len; i++) {
		ret = i2c_send_onebyte(ctx, "Data", send_data[i]);
		if (ret) {
			fprintf(stderr, "Failed send data[%d]: %d\n", i, ret);
			goto out;
		}
	}

out:
	Stop(ctx);

	return ret;
}
