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
	set(unsigned x, unsigned y)
	{
		if (x >= width || y >= height) return;
		const size_t offset = y * width + x;
		buffer_[offset / 8] |= 1 << (offset % 8);
	}

	bool
	get(unsigned x, unsigned y)
	const
	{
		if (x >= width || y >= height) return false;
		const size_t offset = y * width + x;
		return (buffer_[offset / 8] >> (offset % 8)) & 1;
	}

	void
	print(unsigned x, unsigned y, const char* s)
	{
		for (int i = 0; s[i]; ++i) {
			print(x + i * 6, y, s[i]);
		}
	}

	void
	print(unsigned x, unsigned y, char c)
	{
		for (unsigned yy = 0; yy < 7; ++yy) {
			for (unsigned xx = 0; xx < 5; ++xx) {
				if (font[(c - 'A') * 7 + yy] & (0x10 >> xx)) {
					set(x + xx, y + yy);
				}
			}
		}
	}

private:
	uint8_t buffer_[width * height / 8];
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
	: nextRowScanned_()
	{
		red_.print(1, 1, "SZERETLEK");
		green_.print(1, 9, "JULI");
		for (int i = 2; i < 14; ++i) {
			pinMode(i, OUTPUT);
			digitalWrite(i, LOW);
		}
	}

	void
	scan()
	{
		for (int x = 0; x < 8 * 8; ++x) {
			unsigned pixel = (PINB & 0xf0);
			pixel |= green_.get(x, nextRowScanned_);
			pixel |= green_.get(x, nextRowScanned_ + 16) << 1;
			pixel |= red_.get(x, nextRowScanned_) << 2;
			pixel |= red_.get(x, nextRowScanned_ + 16) << 3;
			digitalWrite(clk, LOW);
			PORTB = pixel;
			digitalWrite(clk, HIGH);
		}
		digitalWrite(oe, HIGH);
		PORTD = (PIND & B11000011) | (nextRowScanned_ << 2);
		digitalWrite(lat, HIGH);
		digitalWrite(lat, LOW);
		digitalWrite(oe, LOW);
		nextRowScanned_ = (nextRowScanned_ + 1) & 0xf;
	}

private:
	unsigned nextRowScanned_;
	Bitmap red_, green_;
};

static Display disp;

void setup() {}

void
loop()
{
	disp.scan();
}
