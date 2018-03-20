#ifndef LEDMATRIX_MATH_H
#define LEDMATRIX_MATH_H

#include <cstdint>
#include <array>
#include <algorithm>
#include <cmath>
#include <cassert>

struct Table {
	static constexpr uint8_t pi() { return 128; }

	template <class Generator>
	Table(Generator g, int16_t mid = 0)
	: mid_(mid)
	{
		std::generate(tab_.begin(), tab_.end(), [g, n = 0]() mutable { return g(std::sin(n++ * M_PI / pi())); });
	}

	int16_t cos(uint8_t phi) const { return sin(phi + Table::pi() / 2); }

	int16_t sin(uint8_t phi) const {
		return (phi < 128) ? lookup(phi) : (mid_ - lookup(phi - 128));
	}

private:
	int16_t lookup(uint8_t phi) const {
		assert(phi < 128);
		return (phi <= 64) ? tab_[phi] : tab_[128 - phi];
	}

	std::array<int16_t, 65> tab_;
	uint16_t mid_;
};

struct Math {
	Math()
	: dtab_([](double sin) { return 256.0 * sin; })
	, p0tab_([](double sin) { return 128 + int16_t(256.0 * 47.5 * (1.0 + sqrt(2.0) * sin)); }, 24576)
	{
	}

	int16_t u0(uint8_t phi) const { return p0tab_.cos(phi + Table::pi() / 4 * 5); }
	int16_t v0(uint8_t phi) const { return p0tab_.sin(phi + Table::pi() / 4 * 5); }
	int16_t du(uint8_t phi) const { return dtab_.cos(phi); }
	int16_t dv(uint8_t phi) const { return dtab_.sin(phi); }

private:
	const Table dtab_;
	const Table p0tab_;
};

#endif // LEDMATRIX_MATH_H
