#include <msp430afe253.h>

int
main()
{
	WDTCTL = WDTPW | WDTHOLD;
	P1DIR = 0x01;
	P1OUT = 0x00;

	for (;;) {
		P1OUT ^= 0x01;
		for (int i = 0; i < 20000; ++i);
	}
}
