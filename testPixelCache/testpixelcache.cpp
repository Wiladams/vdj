#include "gui.h"
#include "screensnapshot.hpp"
#include "normalizedwindow.hpp"

#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_renderer_base.h"
#include "agg_renderer_scanline.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_dda_line.h"
#include "agg_conv_curve.h"
#include "agg_scanline_p.h"
#include "GFont.h"

using namespace vdj;
using namespace alib;
using namespace agg;

constexpr size_t CANVAS_WIDTH = 800;
constexpr size_t CANVAS_HEIGHT = 600;
constexpr size_t square_size = 600;

template <class Ren>
void draw_black_frame(Ren& ren)
{
	size_t i;
	rgba8 c(0, 0, 0);

	for (i = 0; i < ren.height(); ++i)
	{
		ren.copy_pixel(0, i, c);
		ren.copy_pixel(ren.width() - 1, i, c);
	}
	for (i = 0; i < ren.width(); ++i)
	{
		ren.copy_pixel(i, 0, c);
		ren.copy_pixel(i, ren.height() - 1, c);
	}
}

void drawUsingPixelFormat()
{
	rendering_buffer rbuf(gAppSurface->row<uint8_t>(0), canvasWidth, canvasHeight, canvasWidth * 4);
	rbuf.clear(255);

	// Draw outer black frame
	pixfmt_bgra32 pixf(rbuf);


	draw_black_frame(pixf);

	// Create a sub-region of the larger buffer
	rbuf.attach(gAppSurface->row<uint8_t>(0) + (canvasWidth * 4 * 20) +
		4 * 20,				// x-offset
		canvasWidth - 40,
		canvasHeight - 40,
		-canvasWidth * 4);

	// Draw a simple diagonal line
	unsigned i;
	for (i = 0; i < rbuf.height() / 2; i++)
	{
		pixf.copy_pixel(i, i, agg::rgba8(127, 200, 98));
	}

	// Draw outer black frame
	draw_black_frame(pixf);
}

void drawSpectrum()
{
	rendering_buffer rbuf(gAppSurface->row<uint8_t>(0), canvasWidth, canvasHeight, canvasWidth * 4);
	rbuf.clear(255);
	pixfmt_bgra32 pixf(rbuf);

	rgba8 span[CANVAS_WIDTH];

	size_t i;
	for (i = 0; i < CANVAS_WIDTH; i++)
	{
		agg::rgba c(380.0 + 400.0 * i / CANVAS_WIDTH, 0.8);
		span[i] = agg::rgba8(c);
	}

	for (i = 0; i < CANVAS_HEIGHT; i++)
	{
		pixf.blend_color_hspan(0, i, CANVAS_WIDTH, span, nullptr, 255);
	}
}

//Interpolate between colors
struct color_interpolator_rgba8
{
	dda_line_interpolator<16> m_r;
	dda_line_interpolator<16> m_g;
	dda_line_interpolator<16> m_b;
	dda_line_interpolator<16> m_a;

	color_interpolator_rgba8(agg::rgba8 c1, agg::rgba8 c2, size_t len)
		:m_r(c1.r, c2.r, len)
		, m_g(c1.g, c2.g, len)
		, m_b(c1.b, c2.b, len)
		, m_a(c1.a, c2.a, len) {}

	void operator++()
	{
		++m_r; ++m_g; ++m_b; ++m_a;
	}

	rgba8 color() const
	{
		return rgba8(m_r.y(), m_g.y(), m_b.y(), m_a.y());
	}
};

template <typename Renderer>
void color_square_rgba8(Renderer& r, int x, int y, int size,
	rgba8 c1, rgba8 c2, rgba8 c3, rgba8 c4)
{
	int i, j;
	color_interpolator_rgba8 cy1(c1, c4, size);
	color_interpolator_rgba8 cy2(c2, c3, size);

	for (i = 0; i < size; i++)
	{
		color_interpolator_rgba8 cx(cy1.color(), cy2.color(), size);
		for (j = 0; j < size; j++)
		{
			r.copy_pixel(x + j, y + i, cx.color());
			++cx;
		}

		++cy1;
		++cy2;
	}
}


void drawInterpolator()
{
	rendering_buffer rbuf(gAppSurface->row<uint8_t>(0), canvasWidth, canvasHeight, -canvasWidth * 4);

	pixfmt_bgra32 pf(rbuf);
	agg::renderer_base<pixfmt_bgra32> rbase(pf);

	color_square_rgba8(rbase, 0, 0, square_size,
		rgba8(0xc6, 0, 0),
		rgba8(0xc6, 0, 0xff),
		rgba8(0xc6, 0xff, 0xff),
		rgba8(0xc6, 0xfe, 0));
}




template<class Rasterizer, class Renderer, class Scanline, class CharT>
void render_text(double x, double y, Rasterizer& ras, Renderer& ren, Scanline& sl,
	GFont& font, const CharT* str,
	bool hinted = true)
{
	while (*str)
	{
		//font.initGlyph(gl, *str++, hinted);
		GGlyph &gl = font.getGlyph(*str++);
		gl.start_point(x, y);

		conv_curve<GGlyph> curve(gl);

		ras.add_path(curve);
		agg::render_scanlines(ras, sl, ren);
		x += gl.inc_x();
		y += gl.inc_y();
	}
}

// Font parameters
constexpr int fontHeight = 24;
constexpr bool bold = false;
constexpr bool italic = false;

GFontFace timesnewroman(L"Times New Roman", fontHeight, bold, italic);
GFontFace segoe(L"Segoe UI", fontHeight, bold, italic);
GFontFace stencil(L"Stencil", fontHeight, bold, italic);

void drawText()
{
	rendering_buffer rbuf(gAppSurface->row<uint8_t>(0), canvasWidth, canvasHeight, canvasWidth * 4);

	pixfmt_bgra32 pixf(rbuf);
	renderer_base<pixfmt_bgra32> rbase(pixf);
	renderer_scanline_aa_solid<renderer_base<pixfmt_bgra32> > ren(rbase);
	rasterizer_scanline_aa<> ras;
	scanline_p8 sl;

	rbase.clear(rgba8(255, 255, 255));

	if (!stencil.isValid())
		return;

	ren.color(rgba8(0, 0, 0));

	GFont font{};
	stencil.getFont(font, true);

	//double w = 0.0;
	//double h = font.height();
	//measure_text(&w, &h, font, "Application Window", false);

	render_text(10, 100, ras, ren, sl, font, "Application Window");
	render_text(10, 200, ras, ren, sl, font, "Good Morning Sunshine!");

	GFont segoeFont{};
	segoe.getFont(segoeFont, true);
	render_text(10, 300, ras, ren, sl, segoeFont, "institutions");
	render_text(10, 400, ras, ren, sl, segoeFont, "iiiiiiiiiiii");

}

void setup()
{
	setCanvasSize(CANVAS_WIDTH, CANVAS_HEIGHT);

	//drawUsingPixelFormat();
	//drawSpectrum();
	//drawInterpolator();

	drawText();
}
