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
			const double phi = n++ * M_PI / pi();
			return 128.0 + 256.0 * 47.501 * (1.0 + sqrt(2) * sin(phi+5*M_PI/4));
		});
	}

	int16_t u0(uint8_t phi) const { return v0((pi() / 2 + phi) & 255); }
	int16_t v0(uint8_t phi) const { return p0tab_[phi]; }
	int16_t du(uint8_t phi) const { return dv((pi() / 2 + phi) & 255); }
	int16_t dv(uint8_t phi) const { return d(phi); }

private:
	int16_t d(uint8_t phi) const {
		return (phi < 128) ? d2(phi) : -d2(phi - 128);
	}

	int16_t d2(uint8_t phi) const {
		assert(phi < 128);
		return (phi <= 64) ? dtab_[phi] : dtab_[127 - phi];
	}

	std::array<int16_t, 65> dtab_;
	std::array<int16_t, 256> p0tab_;
};

#endif // LEDMATRIX_MATH_H
