#include "math.h"

#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>

#include <sstream>

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
