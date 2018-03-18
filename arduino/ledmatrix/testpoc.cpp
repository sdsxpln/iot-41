#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>

#include <cmath>
#include <sstream>
#include <algorithm>

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

TEST_GROUP(Math) {};

TEST(Math, Rotation) {
	static const struct {
		uint8_t phi;
		int16_t u0;
		int16_t v0;
		int16_t du;
		int16_t dv;
	} vectors[] = {
		  { 0 * Math::pi() / 2,  0 * 256 + 127,  0 * 256 + 128,  256,    0 }
		, { 1 * Math::pi() / 2, 95 * 256 + 128,  0 * 256 + 127,    0,  256 }
		, { 2 * Math::pi() / 2, 95 * 256 + 128, 95 * 256 + 128, -256,    0 }
		, { 3 * Math::pi() / 2,  0 * 256 + 128, 95 * 256 + 128,    0, -256 }
	};

	const Math m;

	for (const auto& v : vectors) {
		std::ostringstream oss;
		oss << "phi=" << int(v.phi);
		const auto text = oss.str();
		CHECK_EQUAL_TEXT(v.u0, m.u0(v.phi), text.c_str());
		CHECK_EQUAL_TEXT(v.v0, m.v0(v.phi), text.c_str());
		CHECK_EQUAL_TEXT(v.du, m.du(v.phi), text.c_str());
		CHECK_EQUAL_TEXT(v.dv, m.dv(v.phi), text.c_str());
	}
}

int
main(int argc, char* argv[])
{
	return CommandLineTestRunner::RunAllTests(argc, argv);
}
