#ifndef LEDMATRIX_MATH_H
#define LEDMATRIX_MATH_H

#include <cstdint>
#include <array>
#include <algorithm>
#include <cmath>
#include <cassert>

struct Table {
	template <class G>
	Table(G generator, int16_t mid = 0)
	: mid_(mid)
	{
		std::generate(tab_.begin(), tab_.end(), generator);
	}

	int16_t lookup(uint8_t phi) const {
		return (phi < 128) ? lookup2(phi) : (mid_ - lookup2(phi - 128));
	}

private:
	int16_t lookup2(uint8_t phi) const {
		assert(phi < 128);
		return (phi <= 64) ? tab_[phi] : tab_[128 - phi];
	}

	std::array<int16_t, 65> tab_;
	uint16_t mid_;
};

struct Math {
	static constexpr uint8_t pi() { return 128; }

	Math()
	: dtab_([n = 0]() mutable { return 256.0 * std::sin(n++ * M_PI / pi()); })
	, p0tab_([n = 0]() mutable { return 128 + int16_t(256.0 * 47.5 * (1.0 + sqrt(2) * sin(n++ * M_PI / pi()))); }, 24576)
	{
	}

	int16_t u0(uint8_t phi) const { return v0(phi + pi() / 2); }
	int16_t v0(uint8_t phi) const { return p0tab_.lookup(phi + pi() / 4 * 5); }
	int16_t du(uint8_t phi) const { return dv(phi + pi() / 2); }
	int16_t dv(uint8_t phi) const { return dtab_.lookup(phi); }

private:
	Table dtab_;
	Table p0tab_;
};

#endif // LEDMATRIX_MATH_H
