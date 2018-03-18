#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>

#include <cmath>
#include <sstream>
#include <algorithm>

struct Math {
	Math() {
		std::generate(std::begin(sintab), std::end(sintab), [phi = 0.0]() mutable {
			const auto p = phi;
			phi += M_PI / 128;
			return 256.0 * std::sin(p);
		});
	}

	int16_t u0(double phi) const {
		return 128.0 + 256.0 * 47.501 * (1.0 + sqrt(2) * cos(phi+5*M_PI/4));
	}

	int16_t v0(double phi) const {
		return 128.0 + 256.0 * 47.501 * (1.0 + sqrt(2) * sin(phi+5*M_PI/4));
	}

	int16_t du(double phi) const { return sintab[uint8_t(64.0 + 128.0 * phi / M_PI)]; }
	int16_t dv(double phi) const { return sintab[uint8_t(128.0 * phi / M_PI)]; }

private:
	std::array<int16_t, 256> sintab;
};

TEST_GROUP(Math) {};

TEST(Math, Rotation) {
	static const struct {
		double phi;
		int16_t u0;
		int16_t v0;
		int16_t du;
		int16_t dv;
	} vectors[] = {
		  { 0.0 * M_PI / 2.0,  0 * 256 + 127,  0 * 256 + 128,  256,    0 }
		, { 1.0 * M_PI / 2.0, 95 * 256 + 128,  0 * 256 + 127,    0,  256 }
		, { 2.0 * M_PI / 2.0, 95 * 256 + 128, 95 * 256 + 128, -256,    0 }
		, { 3.0 * M_PI / 2.0,  0 * 256 + 128, 95 * 256 + 128,    0, -256 }
	};

	const Math m;

	for (const auto& v : vectors) {
		std::ostringstream oss;
		oss << "phi=" << v.phi;
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
