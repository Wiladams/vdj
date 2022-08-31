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


using namespace vdj;
using namespace alib;
using namespace agg;

constexpr size_t CANVAS_WIDTH = 800;
constexpr size_t CANVAS_HEIGHT = 600;
constexpr size_t square_size = 200;

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
	rendering_buffer rbuf(gAppSurface->row_ptr(0), canvasWidth, canvasHeight, canvasWidth * 4);
	//rbuf.clear(255);

	// Draw outer black frame
	pixfmt_bgra32 pixf(rbuf);


	draw_black_frame(pixf);

	// Create a sub-region of the larger buffer
	rbuf.attach(gAppSurface->row_ptr(0) + (canvasWidth * 4 * 20) +
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
	rendering_buffer rbuf(gAppSurface->row_ptr(0), canvasWidth, canvasHeight, canvasWidth * 4);
	//rbuf.clear(255);
	pixfmt_bgra32 pixf(rbuf);

	rgba8 span[CANVAS_WIDTH];

	size_t i;
	for (i = 0; i < CANVAS_WIDTH; i++)
	{
		span[i] = agg::rgba8(agg::rgba(380.0 + 400.0 * i / CANVAS_WIDTH, 0.8));
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
		return PixelRGBA(m_r.y(), m_g.y(), m_b.y(), m_a.y());
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
	color_square_rgba8(gCtxt.fRendererBase, 0, 0, square_size,
		rgba8(0xc6, 0, 0),
		rgba8(0xc6, 0, 0xff),
		rgba8(0xc6, 0xff, 0xff),
		rgba8(0xc6, 0xfe, 0));
}





// Font parameters
constexpr int fontHeight = 24;
constexpr bool bold = false;
constexpr bool italic = false;

GFontFace timesnewroman(L"Times New Roman", fontHeight, bold, italic);
GFontFace segoeface(L"Segoe UI", fontHeight, bold, italic);
GFontFace stencilface(L"Stencil", fontHeight, bold, italic);

void drawText()
{
	//rendering_buffer rbuf(gAppSurface->row_ptr(0), canvasWidth, canvasHeight, canvasWidth * 4);

	//pixfmt_bgra32 pixf(rbuf);
	//renderer_base<pixfmt_bgra32> rbase(pixf);
	//renderer_scanline_aa_solid<renderer_base<pixfmt_bgra32> > ren(rbase);
	//rasterizer_scanline_aa<> ras;
	//scanline_p8 sl;



	if (!stencilface.isValid())
		return;

	//ren.color(rgba8(0, 0, 0));

	GFont stencilfont{};
	stencilface.getFont(stencilfont, true);

	//double w = 0.0;
	//double h = font.height();
	//measure_text(&w, &h, font, "Application Window", false);

	gCtxt.render_text(10, 100, L"Application Window", stencilfont);
	gCtxt.render_text(10, 200, L"Good Morning Sunshine!", stencilfont);

	GFont segoeFont{};
	segoeface.getFont(segoeFont, true);
	gCtxt.render_text(10, 300, L"institutions", segoeFont);
	gCtxt.render_text(10, 400, L"iiiiiiiiiiii", segoeFont);
}

void onFrame()
{
	gCtxt.clear(PixelRGBA(0, 0, 0));

	drawSpectrum();
	//drawUsingPixelFormat();

	drawInterpolator();

	drawText();

	noLoop();
}

void setup()
{
	setCanvasSize(CANVAS_WIDTH, CANVAS_HEIGHT);

}
