#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>

#include <cmath>
#include <sstream>

TEST_GROUP(Math) {};

TEST(Math, Rotation) {
	const auto pi = std::acos(-1.0);
	static const struct {
		double phi;
		int16_t u0;
		int16_t v0;
		int16_t du;
		int16_t dv;
	} vectors[] = {
		  { 0.0 * pi / 2.0,  0 * 256 + 127,  0 * 256 + 128,  256,    0 }
		, { 1.0 * pi / 2.0, 95 * 256 + 128,  0 * 256 + 127,    0,  256 }
		, { 2.0 * pi / 2.0, 95 * 256 + 128, 95 * 256 + 128, -256,    0 }
		, { 3.0 * pi / 2.0,  0 * 256 + 128, 95 * 256 + 128,    0, -256 }
	};

	for (const auto& v : vectors) {
		std::ostringstream oss;
		oss << "phi=" << v.phi;
		const auto text = oss.str();
		CHECK_EQUAL_TEXT(v.u0, int16_t(128.0 + 256.0 * 47.501 * (1.0 + sqrt(2) * cos(v.phi+5*pi/4))), text.c_str());
		CHECK_EQUAL_TEXT(v.v0, int16_t(128.0 + 256.0 * 47.501 * (1.0 + sqrt(2) * sin(v.phi+5*pi/4))), text.c_str());
		CHECK_EQUAL_TEXT(v.du, int16_t(256.0 * cos(v.phi)), text.c_str());
		CHECK_EQUAL_TEXT(v.dv, int16_t(256.0 * sin(v.phi)), text.c_str());
	}
}

int
main(int argc, char* argv[])
{
	return CommandLineTestRunner::RunAllTests(argc, argv);
}
