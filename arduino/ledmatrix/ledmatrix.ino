#include "font5x7.h"

const int la = 2;
const int lb = 3;
const int lc = 4;
const int ld = 5;
const int lat = 6; // st_cp
const int clk = 7; // sh_cp
const int g1 = 8;
const int g2 = 9;
const int r1 = 10;
const int r2 = 11;
const int oe = 13;

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
		for (unsigned y = 0; y < 7; ++y) {
			for (unsigned x = 0; x < w; ++x) {
				if (font5x7[(c - 33) * 7 + y] & (1 << (w - (x + 1)))) {
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
		for (unsigned x = 0; x < 8; ++x) {
			for (unsigned y = 0; y < 7; ++y) {
				if (font5x7[(c - 33) * 7 + y] & (0x80 >> x)) {
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
		for (int i = 2; i < 14; ++i) {
			pinMode(i, OUTPUT);
			digitalWrite(i, LOW);
		}
	}

	void
	scan()
	{
		Bitmap::const_iterator i = bitmap_.beginRow(nextRowToScan_);
		for (int x = 0; x < 64; ++x) {
			digitalWrite(clk, LOW);
			PORTB = (PINB & 0xf0) | *i++;
			digitalWrite(clk, HIGH);
		}
		digitalWrite(oe, HIGH);
		PORTD = (PIND & B11000011) | (nextRowToScan_ << 2);
		digitalWrite(lat, HIGH);
		digitalWrite(lat, LOW);
		digitalWrite(oe, LOW);
		nextRowToScan_ = (nextRowToScan_ + 1) & 0xf;
	}

	Bitmap& bitmap() { return bitmap_; }

private:
	unsigned nextRowToScan_;
	Bitmap bitmap_;
};

static Display disp;

void setup() {}

void
loop()
{
	static unsigned long last = 0;
	static unsigned ctr = 0;
	if (millis() > last + 100) {
		last = millis();
		disp.bitmap().clear();
		disp.bitmap().print(1, 1, false, 32 + (ctr & 63));
		disp.bitmap().printf(1, 9, true, "%lu.%lu", last / 1000, (last / 100) % 10);
		++ctr;
	}
	disp.scan();
}
