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

	void
	print(unsigned x, unsigned y, const char* s, bool color)
	{
		for (int i = 0; s[i]; ++i) {
			print(x + i * 6, y, s[i], color);
		}
	}

	void
	print(unsigned x, unsigned y, char c, bool color)
	{
		for (unsigned yy = 0; yy < 7; ++yy) {
			for (unsigned xx = 0; xx < 5; ++xx) {
				if (font[(c - 'A') * 7 + yy] & (0x10 >> xx)) {
					set(x + xx, y + yy, color);
				}
			}
		}
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
		return (y < height)
			? const_iterator(buffer_ + y * width / 2)
			: end();
	}

private:
	uint8_t buffer_[width * height / 4];
	static const uint8_t font[26 * 7];
};

const uint8_t Bitmap::font[] = {
	  0b01110
	, 0b10001
	, 0b10001
	, 0b11111
	, 0b10001
	, 0b10001
	, 0b10001

	, 0b11110
	, 0b10001
	, 0b10001
	, 0b11110
	, 0b10001
	, 0b10001
	, 0b11110

	, 0b01110
	, 0b10001
	, 0b10000
	, 0b10000
	, 0b10000
	, 0b10001
	, 0b01110

	, 0b11110
	, 0b10001
	, 0b10001
	, 0b10001
	, 0b10001
	, 0b10001
	, 0b11110

	, 0b11111
	, 0b10000
	, 0b10000
	, 0b11110
	, 0b10000
	, 0b10000
	, 0b11111

	, 0b11111
	, 0b10000
	, 0b10000
	, 0b11110
	, 0b10000
	, 0b10000
	, 0b10000

	, 0b01110
	, 0b10001
	, 0b10000
	, 0b10000
	, 0b10011
	, 0b10001
	, 0b01110

	, 0b10001
	, 0b10001
	, 0b10001
	, 0b11111
	, 0b10001
	, 0b10001
	, 0b10001

	, 0b01110
	, 0b00100
	, 0b00100
	, 0b00100
	, 0b00100
	, 0b00100
	, 0b01110

	, 0b00001
	, 0b00001
	, 0b00001
	, 0b00001
	, 0b10001
	, 0b10001
	, 0b01110

	, 0b10001
	, 0b10010
	, 0b10100
	, 0b11000
	, 0b10100
	, 0b10010
	, 0b10001

	, 0b10000
	, 0b10000
	, 0b10000
	, 0b10000
	, 0b10000
	, 0b10000
	, 0b11111

	, 0b10001
	, 0b11011
	, 0b10101
	, 0b10001
	, 0b10001
	, 0b10001
	, 0b10001

	, 0b10001
	, 0b11001
	, 0b10101
	, 0b10011
	, 0b10001
	, 0b10001
	, 0b10001

	, 0b01110
	, 0b10001
	, 0b10001
	, 0b10001
	, 0b10001
	, 0b10001
	, 0b01110

	, 0b11110
	, 0b10001
	, 0b10001
	, 0b11110
	, 0b10000
	, 0b10000
	, 0b10000

	, 0b01110
	, 0b10001
	, 0b10001
	, 0b10001
	, 0b10101
	, 0b10011
	, 0b01111

	, 0b11110
	, 0b10001
	, 0b10001
	, 0b11110
	, 0b10001
	, 0b10001
	, 0b10001

	, 0b01110
	, 0b10001
	, 0b10000
	, 0b01110
	, 0b00001
	, 0b10001
	, 0b01110

	, 0b11111
	, 0b00100
	, 0b00100
	, 0b00100
	, 0b00100
	, 0b00100
	, 0b00100

	, 0b10001
	, 0b10001
	, 0b10001
	, 0b10001
	, 0b10001
	, 0b10001
	, 0b01110

	, 0b10001
	, 0b10001
	, 0b10001
	, 0b10001
	, 0b10001
	, 0b01010
	, 0b00100

	, 0b10001
	, 0b10001
	, 0b10001
	, 0b10001
	, 0b10101
	, 0b10101
	, 0b01010

	, 0b10001
	, 0b10001
	, 0b01010
	, 0b00100
	, 0b01010
	, 0b10001
	, 0b10001

	, 0b10001
	, 0b10001
	, 0b01010
	, 0b00100
	, 0b00100
	, 0b00100
	, 0b00100

	, 0b11111
	, 0b00001
	, 0b00010
	, 0b00100
	, 0b01000
	, 0b10000
	, 0b11111
};

class Display {
public:
	Display()
	: nextRowToScan_()
	{
		bitmap_.print(1, 1, "S", true);
		bitmap_.print(7, 1, "Z", false);
//		bitmap_.print(1, 1, "SZERETLEK", true);
//		bitmap_.print(1, 9, "JULI", false);
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

private:
	unsigned nextRowToScan_;
	Bitmap bitmap_;
};

static Display disp;

void setup() {}

void
loop()
{
	disp.scan();
}
