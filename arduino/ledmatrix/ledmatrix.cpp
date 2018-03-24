//#include "model.h"
#include "font3x5.h"
#include "sample.h"
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
const int r1 = 8;
const int g1 = 9;
const int r2 = 10;
const int g2 = 11;
const int clk = 12; // sh_cp

class Bitmap {
public:
	enum { width = 64, height = 32 };
	enum Color { black, red, green, yellow };

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
	set(int x, int y, Color c)
	{
		if (x < 0 || x >= width || y < 0 || y >= height) return;
		size_t offset = (y * width + x) * 4;
		if (offset >= width * height * 2) {
			offset -= width * height * 2 - 2;
		}
		const uint8_t b = buffer_[offset / 8] & ~(3 << (offset % 8));
		buffer_[offset / 8] = b | (c << (offset % 8));
	}

	void
	print(int x, int y, Color c, const char* s)
	{
		while (*s) {
			print(x, y, c, *s++);
			x += 4;
		}
	}

	void
	print(int x0, int y0, Color c, char ch)
	{
		if (ch < 32 || ch > 96) return;
		const uint8_t* bm = font3x5 + (ch - 32) / 2;
		for (int y = 0; y < 5; ++y) {
			const uint8_t line = pgm_read_byte(bm) >> ((!(ch & 1)) * 4);
			bm += 64 / 2;
			for (int x = 0; x < 3; ++x) {
				set(x0 + x, y0 + y, (line & (1 << (3 - x))) ? c : black);
			}
		}
	}

	void
	printf(int x0, int y0, Color c, const char* fmt, ...)
	{
		va_list ap;
		va_start(ap, fmt);
		char buf[32];
		vsnprintf(buf, sizeof(buf), fmt, ap);
		va_end(ap);
		print(x0, y0, c, buf);
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

struct Display {
	Display()
	: nextRowToScan_()
	{
		DDRB = 0x1f;
		DDRD = 0xfc;
	}

	void
	scan()
	{
		Bitmap::const_iterator i = bitmap_[active_].beginRow(nextRowToScan_);
		for (int x = 0; x < 64; ++x) {
			shift(*i++);
		}
		const uint8_t value = (nextRowToScan_ << 2);
		PORTD = 0xc0 | value; // oe=HIGH lat=HIGH
		PORTD = value; // oe=LOW lat=LOW
		nextRowToScan_ = (nextRowToScan_ + 1) & 0xf;
	}

	void
	shift(uint8_t value)
	{
		PORTB = value; // clk=LOW
		PORTB = 0x10 | value; // clk=HIGH
	}

	Bitmap& bitmap() { return bitmap_[1 - active_]; }

	void swap() { active_ = 1 - active_; }

private:
	unsigned nextRowToScan_;
	Bitmap bitmap_[2];
	int active_{};
};
/*
struct Display2 {
	Display2()
	: nextRowToScan_()
	{
		DDRB = 0x1f;
		DDRD = 0xfc;
	}

	void
	scan()
	{
		Bitmap::const_iterator i = bitmap_[active_].beginRow(nextRowToScan_);
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

	Bitmap& bitmap() { return bitmap_[1 - active_]; }

	void swap() { active_ = 1 - active_; }

private:
	unsigned nextRowToScan_;
};
*/
static Display disp;
static long counter;

ISR(TIMER1_COMPA_vect) { // 16-bit timer: millisecond interrupts
	disp.scan();
	++counter;
}

static void
setup()
{
	TCCR1B = (1 << WGM12) | (1 << CS11);
	const uint16_t ctc_match_overflow = F_CPU / 1000 / 8;
	OCR1AH = ctc_match_overflow >> 8;
	OCR1AL = ctc_match_overflow & 255;

	TIMSK1 = 1 << OCIE1A;
	sei();
}

static void
futar_print_line(int y, int num, const char* station, int mins)
{
	disp.bitmap().printf(0, y, Bitmap::red, "%3d", num);
	disp.bitmap().printf(3 * 4, y, Bitmap::green, station);
	disp.bitmap().printf(14 * 4, y, Bitmap::yellow, "%2d", mins);
}

static void
loop()
{
	disp.bitmap().clear();
	futar_print_line(1, 244, "ORS VEZER T", 7);
	futar_print_line(7, 144, "ORS VEZER T", 12);
	futar_print_line(13, 244, "ORS VEZER T", 13);
	futar_print_line(19, 46, "NYIRPALOTA UT", 20);
	futar_print_line(25, 244, "ORS VEZER T", 25);
	disp.swap();
}

int
main()
{
	setup();
	for (;;) {
		loop();
	}
}
