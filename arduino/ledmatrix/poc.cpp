#define PROGMEM
#define pgm_read_byte *
#include "model.h"

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

template <typename T, std::size_t N>
Point<T, N>&
operator*=(Point<T, N>& lhs, T rhs)
{
	std::transform(lhs.begin(), lhs.end(), lhs.begin(), [rhs](T v){ return rhs * v; });
	return lhs;
}

template <typename T, std::size_t N>
Point<T, N>
operator*(Point<T, N> lhs, T rhs)
{
	return lhs *= rhs;
}

using Point2i = Point<int, 2>;

using SdlWindow = std::unique_ptr<SDL_Window, void(*)(SDL_Window*)>;

struct Sdl {
	Sdl() { if(SDL_Init(SDL_INIT_VIDEO) < 0) throw std::runtime_error("SDL_Init"); }
	Sdl(const Sdl&) = delete;
	Sdl& operator=(const Sdl&) = delete;
	~Sdl() { SDL_Quit(); }

	SdlWindow
	createWindow(const char* title, Point2i dim)
	{
		const auto u = SDL_WINDOWPOS_UNDEFINED;
		SDL_Window* window = SDL_CreateWindow(title, u, u, dim[0], dim[1], SDL_WINDOW_SHOWN);
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

static const int cellsize = 10;

struct SdlPixelPairRenderPolicy {
	SdlPixelPairRenderPolicy(SdlRenderer& rend) : rend_(rend) {}

	void
	putPixelPair(int16_t x, int16_t y, uint8_t c)
	const
	{
		putPixel({x, y}, c & 3);
		putPixel({x, y + 16}, (c >> 2) & 3);
	}

private:
	void
	putPixel(Point2i p, uint8_t c)
	const
	{
		rend_.setDrawColor(getColor(c));
		rend_.fillRect(SDL_Rect{p[0] * cellsize + 1, p[1] * cellsize + 1, cellsize - 1, cellsize - 1});
	}

	static SdlColor
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

	SdlRenderer& rend_;
};

using SdlModel = Model<SdlPixelPairRenderPolicy>;

struct SdlView {
	SdlView(SdlModel& model, SdlRenderer& rend) : model_(model), rend_(rend) {}

	void
	render()
	{
		rend_.setDrawColor(SdlColor::black());
		rend_.clear();
		model_.show();
		rend_.present();
	}

	static Point2i dimensions() { return Point2i{ 64, 32 } * cellsize + Point2i{ 1, 1 }; }

private:
	SdlModel& model_;
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
	SdlControl(SdlModel& model) : model_(model) {}

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
		}
	}

	SdlModel& model_;
	bool quit_{};
};

int
main()
{
	Sdl sdl;
	auto window = sdl.createWindow("ledmatrix", SdlView::dimensions());
	SdlRenderer rend(window);
	SdlPixelPairRenderPolicy spprp(rend);
	SdlModel model(spprp);
	SdlControl control(model);
	SdlView view(model, rend);
	for (int i = 0; !control.shouldQuit(); ++i) {
		control.update();
		model.update();
		view.render();
	}
}
