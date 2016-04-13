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

void
setup()
{
	for (int i = 2; i < 14; ++i) {
		pinMode(i, OUTPUT);
		digitalWrite(i, LOW);
	}
}

void
loop()
{
	static unsigned row = 0;
	static unsigned long last = 0;
	static unsigned col = 0;
	static int delta = 1;

	if (millis() > last + 20) {
		last = millis();
		col += delta;
		if (!col || col == 63) delta = -delta;
	}
	for (int i = 0; i < 8 * 8; ++i) {
		digitalWrite(clk, LOW);
		PORTB = (PINB & B11110000) | ((i == col) ? 9 : 0);
//		digitalWrite(r1, i < 1);
//		digitalWrite(r2, i < 1);
//		digitalWrite(g1, i < 1);
//		digitalWrite(g2, i < 1);
		digitalWrite(clk, HIGH);
	}
	digitalWrite(oe, HIGH);
	PORTD = (PIND & B11000011) | (row << 2);
//	digitalWrite(la, row & 1);
//	digitalWrite(lb, row & 2);
//	digitalWrite(lc, row & 4);
//	digitalWrite(ld, row & 8);
	digitalWrite(lat, LOW);
	digitalWrite(lat, HIGH);
	digitalWrite(lat, LOW);
	digitalWrite(oe, LOW);
	row = (row + 1) & 0xf;
}
