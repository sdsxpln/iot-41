#ifndef LEDMATRIX_MODEL_H
#define LEDMATRIX_MODEL_H

#include "sample.h"
#include "math.h"

#include <cstdint>

template <class PixelPairRenderPolicy>
struct Model {
	Model(PixelPairRenderPolicy& rend)
	: rend_(rend)
	{
		update();
	}

	void
	update()
	{
		phi_ += delta();
		du_ = math_.du(phi_);
		dv_ = math_.dv(phi_);
		u0_ = math_.u0(phi_);
		v0_ = math_.v0(phi_);
	}

	void rotateLeft() { --state_; }
	void rotateRight() { ++state_; }

	void
	show()
	{
		for (int y = 0; y != 16; ++y) {
			scan();
		}
	}

	void
	scan()
	{
		if (!y_) {
			u1_ = u0_;
			v1_ = v0_;
			u2_ = u0_ - (dv_ << 4);
			v2_ = v0_ + (du_ << 4);
		}
		int16_t u1 = u1_;
		int16_t v1 = v1_;
		int16_t u2 = u2_;
		int16_t v2 = v2_;
		for (int x = 0; x != 64; ++x) {
			rend_.putPixelPair(x, y_, lookup(u1 >> 8, v1 >> 8) | (lookup(u2 >> 8, v2 >> 8) << 2));
			u1 += du_;
			v1 += dv_;
			u2 += du_;
			v2 += dv_;
		}
		u1_ -= dv_;
		v1_ += du_;
		u2_ -= dv_;
		v2_ += du_;
		y_ = (y_ + 1) & 15;
	}

private:
	uint8_t
	lookup(uint8_t u, uint8_t v)
	const
	{
		if (u >= 96 || v >= 96) return 0;
		return pgm_read_byte(sample_data + (v << 7) + u);
	}

	int16_t desiredPhi() const { return state_ * Table::pi() / 2; }

	int16_t delta() const {
		const int16_t d = desiredPhi() - phi_;
		const int16_t increment = d / 4;
		return increment ? increment : d;
	}

	const Math math_;
	PixelPairRenderPolicy& rend_;
	int16_t phi_{};
	int state_{};
	int16_t du_, dv_;
	int16_t u0_, v0_;
	int8_t y_{};
	int16_t u1_, v1_;
	int16_t u2_, v2_;
};

#endif // LEDMATRIX_MODEL_H
