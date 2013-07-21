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
#include "serial.h"

/*
 * serial_init sets up serial communications, and takes two arguments:
 * the bit rate for the serial port (which should be the actual desired
 * bit rate, so 9600 for 9600 bps), and the frame format. If the frame
 * format value is 0, the default of 8N1 is used.
 */
void
serial_init(int baud, int ffmt)
{
	int	ubrr;

	ubrr = SERIAL_SPEED(baud);
	UBRR0H = (unsigned char)(ubrr >> 8);
	UBRR0L = (unsigned char)ubrr;
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);

	if (0 == ffmt)
		ffmt = SERIAL_8N1;
	UCSR0C = ffmt;
}


/*
 * serial_block_transmit_byte sends a single byte. It will block until the
 * transmit buffer is empty, and then write the byte to the USART.
 */
void
serial_block_transmit_byte(unsigned char b)
{
	while (!(UCSR0A & (1 << UDRE0))) ;
	UDR0 = b;
}


/*
 * serial_transmit writes a data buffer to the serial line.
 */
void
serial_transmit(unsigned char *data, unsigned int len)
{
	unsigned int	curbyte = 0;
	while (curbyte < len)
		serial_block_transmit_byte(data[curbyte++]);
}


/*
 * serial_block_receive_byte blocks until the receive buffer is ready, and
 * then returns the byte that was read in.
 */
unsigned char
serial_block_receive_byte()
{
	while (!(UCSR0A & (1 << RXC0))) ;
	return UDR0;
}
