#ifndef LEDMATRIX_MATH_H
#define LEDMATRIX_MATH_H

#include <cstdint>
#include <array>
#include <algorithm>
#include <cmath>

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

	int16_t u0(uint8_t phi) const { return p0tab_[(pi() / 2 + phi) & 255]; }
	int16_t v0(uint8_t phi) const { return p0tab_[phi]; }
	int16_t du(uint8_t phi) const { return dtab_[(pi() / 2 + phi) & 255]; }
	int16_t dv(uint8_t phi) const { return dtab_[phi]; }

private:
	std::array<int16_t, 256> dtab_;
	std::array<int16_t, 256> p0tab_;
};

#endif // LEDMATRIX_MATH_H
