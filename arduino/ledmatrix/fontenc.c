#include <stdio.h>
#include <stdint.h>
#include <assert.h>

int
main()
{
	int c;
	uint8_t b = 0;
	int n = 0;

	while ((c = getchar()) != EOF) {
		assert(!c || c == 1);
		b = (b << 1) | (!c);
		if (!(++n & 7)) putchar(b);
	}
	assert(!(n & 7));
}
