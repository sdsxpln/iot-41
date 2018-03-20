#ifndef LEDMATRIX_MATH_H
#define LEDMATRIX_MATH_H

#include <cstdint>
#include <array>
#include <algorithm>
#include <cmath>
#include <cassert>

struct Math {
	static constexpr uint8_t pi() { return 128; }

	Math() {
		std::generate(dtab_.begin(), dtab_.end(), [n = 0]() mutable {
			return 256.0 * std::sin(n++ * M_PI / pi());
		});
		std::generate(p0tab_.begin(), p0tab_.end(), [n = 0]() mutable {
			return 128 + int16_t(256.0 * 47.5 * (1.0 + sqrt(2) * sin(n++ * M_PI / pi())));
		});
	}

	int16_t u0(uint8_t phi) const { return v0(phi + pi() / 2); }
	int16_t v0(uint8_t phi) const { return lookup(p0tab_, 24576, phi + pi() / 4 * 5); }
	int16_t du(uint8_t phi) const { return dv(phi + pi() / 2); }
	int16_t dv(uint8_t phi) const { return lookup(dtab_, 0, phi); }

private:
	using Table = std::array<int16_t, 65>;

	static int16_t lookup(const Table& t, int16_t mid, uint8_t phi) {
		return (phi < 128) ? lookup2(t, phi) : (mid - lookup2(t, phi - 128));
	}

	static int16_t lookup2(const Table& t, uint8_t phi) {
		assert(phi < 128);
		return (phi <= 64) ? t[phi] : t[128 - phi];
	}

	Table dtab_;
	Table p0tab_;
};

#endif // LEDMATRIX_MATH_H
