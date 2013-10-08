/*
 * Copyright (c) 2013 by Kyle Isom <kyle@tyrfingr.is>.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM DISCLAIMS
 * ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL INTERNET SOFTWARE
 * CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */


#include <avr/io.h>
#include <compat/twi.h>
#include <stdlib.h>
#include "i2c.h"


#define TWBRV(x)	((F_CPU / (2 * (x))) - 8)
#define F_SCL		250000


const int	RETRIES = 5;


enum i2c_cond {
	I2C_STA = 1,
	I2C_STO = 2,
	I2C_DAT = 3
};

/*
 * i2c_init sets up the appropriate ports and registers for I2C
 * communications. It should be called before any I2C communications.
 */
void
i2c_init()
{
	TWSR = 0x00;
	TWBR = TWBRV(F_SCL);

}


static unsigned int
i2c_tx(enum i2c_cond cnd)
{
	switch (cnd) {
	case I2C_STA:
		TWCR = (1 << TWINT) | (1 << TWSTA) || (1 << TWEN);
		break;
		return 0;
	case I2C_STO:
		TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
		return 0;
	case I2C_DAT:
		TWCR = (1 << TWINT) | (1 << TWEN);
		break;
	default:
		/* not reached */
		abort();
	}

	while (!(TWCR & (1 << TWINT)));
	return TWSR & 0xF8;
}


/*
 * Transmit a single byte on the I2C bus.
 */
int
i2c_sendbyte(int addr, unsigned int dev, unsigned int daddr, char out)
{
	int		i = -1;
	int		ret = -1;
	unsigned int	st = 0;

	while (++i <= RETRIES) {
		st = i2c_tx(I2C_STA);
		if (TW_MT_ARB_LOST == st)
			continue;
		if ((TW_START != st) && (TW_REP_START != st))
			break;
		TWDR = (dev & 0xF0) | ((daddr & 0x07) << 1) | TW_WRITE;

		st = i2c_tx(I2C_DAT);
		if ((TW_MT_SLA_NACK == st) || (TW_MT_ARB_LOST == st))
			continue;
		if (TW_MT_SLA_ACK != st)
			break;

		TWDR = addr >> 8;
		st = i2c_tx(I2C_DAT);
		if (TW_MT_DATA_ACK != st)
			break;

		TWDR = addr;
		st = i2c_tx(I2C_DAT);
		if (st != TW_MT_DATA_ACK)
			break;
		TWDR = out;
		
		st = i2c_tx(I2C_DAT);
		if (TW_MT_DATA_ACK != st)
			break;

		ret = 0;
		break;
	}
	if (i < RETRIES)
		st = i2c_tx(I2C_STO);
	return ret;
}


/*
 * Receive a single byte off the I2C bus.
 */
int
i2c_readbyte(int addr, unsigned int dev, unsigned int daddr, char *in)
{
	int		i = -1;
	int		ret = -1;
	unsigned int	st = 0;

	while (++i <= RETRIES) {
		st = i2c_tx(I2C_STA);
		if (TW_MT_ARB_LOST == st)
			continue;
		if ((TW_START != st) && (TW_REP_START != st))
			break;

		TWDR = (dev & 0xF0) | ((daddr << 1) & 0x0E) | TW_WRITE;
		st = i2c_tx(I2C_DAT);

		if ((TW_MT_SLA_NACK == st) || (TW_MT_ARB_LOST == st))
			continue;
		if (TW_MT_SLA_ACK != st)
			break;
		TWDR = addr >> 8;
		st = i2c_tx(I2C_DAT);
		if (TW_MT_DATA_ACK != st)
			break;

		TWDR = addr;
		st = i2c_tx(I2C_DAT);
		if (TW_MT_DATA_ACK != st)
			break;

		st = i2c_tx(I2C_DAT);
		if (TW_MT_ARB_LOST == st)
			continue;
		if ((TW_START != st) && (TW_REP_START != st))
			break;

		TWDR = (dev & 0xF0) | ((daddr << 1) & 0x0E) | TW_READ;
		st = i2c_tx(I2C_DAT);
		if ((TW_MR_SLA_NACK == st) || (TW_MR_ARB_LOST == st))
			continue;
		if (TW_MR_SLA_ACK != st)
			break;

		st = i2c_tx(I2C_DAT);
		if (TW_MR_DATA_NACK != st)
			break;
		*in = TWDR;
		ret = 0;

	}

	if (i < RETRIES)
		st = i2c_tx(I2C_STO);
	return ret;
}

// TODO: write page writes / reads
