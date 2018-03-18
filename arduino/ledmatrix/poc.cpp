#include <SDL.h>

#include <array>
#include <algorithm>
#include <memory>
#include <cassert>

template <typename T, std::size_t N>
struct Point : std::array<T, N> {
	using Base = std::array<T, N>;

	Point() : Base({}) {}

	template <typename... U>
	Point(U... values) : Base({values...}) {}
};

template <typename T, std::size_t N>
Point<T, N>&
operator+=(Point<T, N>& lhs, const Point<T, N>& rhs)
{
	std::transform(lhs.begin(), lhs.end(), rhs.begin(), lhs.begin(), std::plus<T>());
	return lhs;
}

template <typename T, std::size_t N>
Point<T, N>
operator+(Point<T, N> lhs, const Point<T, N>& rhs)
{
	return lhs += rhs;
}

using Point2i = Point<int, 2>;

using SdlWindow = std::unique_ptr<SDL_Window, void(*)(SDL_Window*)>;

struct Sdl {
	Sdl() { if(SDL_Init(SDL_INIT_VIDEO) < 0) throw std::runtime_error("SDL_Init"); }
	Sdl(const Sdl&) = delete;
	Sdl& operator=(const Sdl&) = delete;
	~Sdl() { SDL_Quit(); }

	SdlWindow
	createWindow(const char* title, int w, int h)
	{
		const auto u = SDL_WINDOWPOS_UNDEFINED;
		SDL_Window* window = SDL_CreateWindow(title, u, u, w, h, SDL_WINDOW_SHOWN);
		if (!window) throw std::runtime_error("SDL_CreateWindow");
		return SdlWindow(window, SDL_DestroyWindow);
	}
};

struct SdlColor {
	uint8_t r, g, b, opacity;

	SdlColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t opacity = SDL_ALPHA_OPAQUE)
	: r(red), g(green), b(blue), opacity(opacity)
	{}

	static SdlColor black() { return SdlColor(0, 0, 0); }
	static SdlColor gray() { return SdlColor(32, 32, 32); }
	static SdlColor cyan() { return SdlColor(0, 255, 255); }
	static SdlColor yellow() { return SdlColor(255, 255, 0); }
	static SdlColor purple() { return SdlColor(128, 0, 128); }
	static SdlColor green() { return SdlColor(0, 255, 0); }
	static SdlColor red() { return SdlColor(255, 0, 0); }
	static SdlColor blue() { return SdlColor(0, 0, 255); }
	static SdlColor orange() { return SdlColor(255, 165, 0); }
};

struct SdlRenderer {
	SdlRenderer(SdlWindow& window)
	: rend_(SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED), SDL_DestroyRenderer)
	{
		if (!rend_) throw std::runtime_error("SDL_CreateRenderer");
	}

	void setDrawColor(SdlColor c) { SDL_SetRenderDrawColor(rend_.get(), c.r, c.g, c.b, c.opacity); }
	void clear() { SDL_RenderClear(rend_.get()); }
	void drawLine(Point2i a, Point2i b) { SDL_RenderDrawLine(rend_.get(), a[0], a[1], b[0], b[1]); }
	void present() { SDL_RenderPresent(rend_.get()); }
	void fillRect(SDL_Rect r) { SDL_RenderFillRect(rend_.get(), &r); }

private:
	std::unique_ptr<SDL_Renderer, void(*)(SDL_Renderer*)> rend_;
};

#include "sample.h"

struct Model {
	Model()
	{
		std::copy(std::begin(sample_data), std::end(sample_data), frameBuffer_.begin());
	}

	char
	operator()(int u, int v)
	const
	{
		if (u < 0 || u >= 96 || v < 0 || v >= 96) return 0;
		return frameBuffer_[v * 96 + u];
	}

	void
	update()
	{
		phi_ += (desiredPhi() - phi_) / 3.0;
	}

	double phi() const { return phi_; }
	void rotateLeft() { --state_; }
	void rotateRight() { ++state_; }

private:
	double desiredPhi() const { return state_ * std::acos(-1.0) / 2.0; }

	std::array<char, 96 * 96> frameBuffer_;
	double phi_{};
	int state_{};
};

struct SdlView {
	SdlView(const Model& model, SdlRenderer& rend) : model_(model), rend_(rend) {}

	void
	render()
	{
		rend_.setDrawColor(SdlColor::black());
		rend_.clear();
		render(model_);
		rend_.present();
	}

	static int width() { return 64 * cellsize + 1; }
	static int height() { return 32 * cellsize + 1; }

private:
	static const int cellsize = 10;

	void
	render(const Model& m)
	{
		const auto pi = std::acos(-1.0);
		const auto phi = m.phi();
		const int16_t u0 = 128 + 256 * 47.501 * (1.0 + std::sqrt(2) * std::cos(phi+5*pi/4));
		const int16_t v0 = 128 + 256 * 47.501 * (1.0 + std::sqrt(2) * std::sin(phi+5*pi/4));
		const int16_t du = 256 * std::cos(phi);
		const int16_t dv = 256 * std::sin(phi);

		int16_t u = u0;
		int16_t v = v0;
		for (int y = 0; y != 32; ++y) {
			int16_t um = u;
			int16_t vm = v;
			for (int x = 0; x != 64; ++x) {
				rend_.setDrawColor(getColor(m(um >> 8, vm >> 8)));
				rend_.fillRect(SDL_Rect{x * cellsize + 1, y * cellsize + 1, cellsize - 1, cellsize - 1});
				um += du;
				vm += dv;
			}
			u -= dv;
			v += du;
		}
	}

	SdlColor
	getColor(char c)
	{
		switch (c) {
		case 0: return SdlColor::black();
		case 1: return SdlColor::red();
		case 2: return SdlColor::green();
		case 3: return SdlColor::yellow();
		}
		assert(!"invalid cell");
	}

	const Model& model_;
	SdlRenderer& rend_;
};

struct SdlEvents {
	struct iterator {
		iterator(SdlEvents* se) : se_(se) {}

		bool operator!=(const iterator& rhs) const { return se_ != rhs.se_; }
		iterator& operator++() { assert(se_); se_ = se_->next(); return *this; }
		SDL_Event& operator*() { assert(se_); return se_->ev_; }

	private:
		SdlEvents* se_;
	};

	iterator begin() { return iterator(next()); }
	iterator end() { return iterator(nullptr); }

private:
	struct Deadline {
		unsigned remaining() const { return std::max(0u, value_ - SDL_GetTicks()); }
		bool have_time() const { return value_ > SDL_GetTicks(); }

	private:
		uint32_t value_ = SDL_GetTicks() + 40;
	};

	bool hasNext() { return dl_.have_time() && SDL_WaitEventTimeout(&ev_, dl_.remaining()); }
	SdlEvents* next() { return hasNext() ? this : nullptr; }

	const Deadline dl_;
	SDL_Event ev_;
};

static void
requestSdlQuit()
{
	SDL_Event e;
	e.type = SDL_QUIT;
	e.quit.timestamp = SDL_GetTicks();
	SDL_PushEvent(&e);
}

struct SdlControl {
	SdlControl(Model& model) : model_(model) {}

	void update() { for (const auto& ev : SdlEvents()) process(ev); }

	bool shouldQuit() const { return quit_; }

private:
	void
	process(const SDL_Event& ev)
	{
		if (ev.type == SDL_QUIT) {
			quit_ = true;
		} else if (ev.type == SDL_KEYDOWN) {
			process(ev.key);
		}
	}

	void
	process(const SDL_KeyboardEvent& kev)
	{
		switch (kev.keysym.sym) {
		case SDLK_ESCAPE:
		case SDLK_q:
			requestSdlQuit();
			break;
		case SDLK_LEFT: model_.rotateLeft(); break;
		case SDLK_RIGHT: model_.rotateRight(); break;
/*		case SDLK_DOWN: model_.drop(); break;
		case SDLK_UP: model_.rotate(); break;*/
		}
	}

	Model& model_;
	bool quit_{};
};

int
main()
{
	Sdl sdl;
	auto window = sdl.createWindow("ledmatrix", SdlView::width(), SdlView::height());
	SdlRenderer rend(window);
	Model model;
	SdlControl control(model);
	SdlView view(model, rend);
	for (int i = 0; !control.shouldQuit(); ++i) {
		control.update();
		if (!(i & 1)) model.update();
		view.render();
	}
}
