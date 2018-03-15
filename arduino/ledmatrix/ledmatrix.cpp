#include "font5x7.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

//portd:0-7
//portb:8-15

// slow portd
const int la = 2;
const int lb = 3;
const int lc = 4;
const int ld = 5;
const int lat = 6; // st_cp | stb
const int oe = 7;

// fast portb
const int g1 = 8;
const int g2 = 9;
const int r1 = 10;
const int r2 = 11;
const int clk = 12; // sh_cp

// oe 13 -> 7
// clk 7 -> 12

class Bitmap {
public:
	enum { width = 64, height = 32 };

	Bitmap()
	{
		clear();
	}

	void
	clear()
	{
		memset(buffer_, 0, sizeof(buffer_));
	}

	void
	set(unsigned x, unsigned y, bool color)
	{
		if (x >= width || y >= height) return;
		size_t offset = (y * width + x) * 4;
		if (offset >= width * height * 2) {
			offset -= width * height * 2 - 1;
		}
		offset += color * 2;
		buffer_[offset / 8] |= 1 << (offset % 8);
	}

	unsigned
	print(unsigned x, unsigned y, bool color, const char* s)
	{
		unsigned w = 0;
		for (int i = 0; s[i]; ++i) {
			w += print(x + w, y, color, s[i]);
		}
		return w;
	}

	unsigned
	print(unsigned x0, unsigned y0, bool color, char c)
	{
		if (c < 33 || c > 95) return 3;
		const unsigned w = getWidthOf(c);
		const uint8_t* bm = font5x7 + (c - 33) * 7;
		for (unsigned y = 0; y < 7; ++y, ++bm) {
			unsigned mask = 1 << (w - 1);
			for (unsigned x = 0; x < w; ++x, mask >>= 1) {
				if (*bm & mask) {
					set(x0 + x, y0 + y, color);
				}
			}
		}
		return w + 1;
	}

	unsigned
	printf(unsigned x0, unsigned y0, bool color, const char* fmt, ...)
	{
		va_list ap;
		va_start(ap, fmt);
		char buf[32];
		vsnprintf(buf, sizeof(buf), fmt, ap);
		va_end(ap);
		return print(x0, y0, color, buf);
	}

	unsigned
	getWidthOf(char c)
	{
		if (c < 33 || c > 95) return 0;
		const uint8_t *const bm = font5x7 + (c - 33) * 7;
		for (unsigned x = 0; x < 8; ++x) {
			for (unsigned y = 0; y < 7; ++y) {
				if (bm[y] & (0x80 >> x)) {
					return 8 - x;
				}
			}
		}
		return 0;
	}

	class const_iterator {
	public:
		const_iterator(const uint8_t* p)
		: p_(p)
		, shift_()
		{}

		const_iterator
		operator++()
		{
			shift_ += 4;
			if (shift_ == 8) {
				shift_ = 0;
				++p_;
			}
			return *this;
		}

		const_iterator
		operator++(int)
		{
			const_iterator result = *this;
			operator++();
			return result;
		}

		unsigned
		operator*()
		const
		{
			return (*p_ >> shift_) & 15;
		}

	private:
		const uint8_t* p_;
		unsigned shift_;
	};

	const_iterator
	begin()
	const
	{
		return const_iterator(buffer_);
	}

	const_iterator
	end()
	const
	{
		return const_iterator(buffer_ + sizeof(buffer_));
	}

	const_iterator
	beginRow(unsigned y)
	{
		return (y < height / 2)
			? const_iterator(buffer_ + y * width / 2)
			: end();
	}

private:
	uint8_t buffer_[width * height / 4];
};

class Display {
public:
	Display()
	: nextRowToScan_()
	{
		DDRB = 0x1f;
		DDRD = 0xfc;
	}

	void
	scan()
	{
		Bitmap::const_iterator i = bitmap_.beginRow(nextRowToScan_);
		for (int x = 0; x < 64; ++x) {
			const uint8_t value = *i++;
			PORTB = value; // clk=LOW
			PORTB = 0x10 | value; // clk=HIGH
		}
		const uint8_t value = (nextRowToScan_ << 2);
		PORTD = 0xc0 | value; // oe=HIGH lat=HIGH
		PORTD = value; // oe=LOW lat=LOW
		nextRowToScan_ = (nextRowToScan_ + 1) & 0xf;
	}

	Bitmap& bitmap() { return bitmap_; }

private:
	unsigned nextRowToScan_;
	Bitmap bitmap_;
};

static Display disp;

void
setup()
{
	const int c = 'A';
	disp.bitmap().printf(1, 1, false, "HELLO %02d:  %c", c, c);
	const int last = 42;
	disp.bitmap().printf(1, 13, true, "%lu.%lu", last / 1000, (last / 100) % 10);
}

void
loop()
{
	disp.scan();
}

int
main()
{
	setup();
	for (;;) {
		loop();
	}
}
