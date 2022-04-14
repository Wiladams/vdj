#pragma once

#include "apphost.h"
#include "agg_basics.h"

#include <map>

// Data structure that holds the curve vertices
// for a specific glyph
struct GGlyph
{
//private:
	double		m_start_x;
	double		m_start_y;
	int			m_inc_x;
	int			m_inc_y;
	uint8_t* m_flags;
	double* m_vertices;
	size_t m_max_vertices;

	// The cursor for iterating vertices
	const uint8_t* m_cur_flag;
	const double* m_cur_vertex;
	
public:
	GGlyph()
		: m_max_vertices(256)
		, m_flags(new uint8_t[256])
		, m_vertices(new double[512])
		, m_cur_flag(m_flags)
		, m_cur_vertex(m_vertices)
		, m_start_x(0.0)
		, m_start_y(0.0)
		, m_inc_x(0)
		, m_inc_y(0)
	{
		m_vertices[0] = m_vertices[1] = 0.0;
		m_flags[0] = agg::path_cmd_stop;
	}

	GGlyph(const GGlyph& other)
		: m_max_vertices(0)
		, m_flags(nullptr)
		, m_vertices(nullptr)
		, m_cur_flag(m_flags)
		, m_cur_vertex(m_vertices)
		, m_start_x(0.0)
		, m_start_y(0.0)
		, m_inc_x(0)
		, m_inc_y(0)
	{
		printf("GGlyph COPY CONSTRUCTOR\n");
		m_start_x = other.m_start_x;
		m_start_y = other.m_start_y;
		m_inc_x = other.m_inc_x;
		m_inc_y = other.m_inc_y;

		m_max_vertices = other.m_max_vertices;
		//reset(m_max_vertices);

		m_flags = new uint8_t[m_max_vertices];
		std::memcpy(m_flags, other.m_flags, m_max_vertices);

		m_vertices = new double[m_max_vertices*2];
		std::memcpy(m_vertices, other.m_vertices, sizeof(double)*m_max_vertices*2);

		rewind(0);
	}

	~GGlyph()
	{
		if (m_vertices != nullptr)
			delete[] m_vertices;

		if (m_flags != nullptr)
			delete[] m_flags;
	}

	void reset(size_t total_size)
	{
		// resize arrays if necessary
		if (m_max_vertices <= total_size / sizeof(POINTFX))
		{
			delete[] m_vertices;
			delete[] m_flags;
			m_max_vertices = total_size / sizeof(POINTFX) + 256;
			m_flags = new uint8_t[m_max_vertices];
			m_vertices = new double[m_max_vertices * 2];
		}

		m_vertices[0] = m_vertices[1] = 0.0;
		m_flags[0] = agg::path_cmd_stop;
		rewind(0);
	}

	int inc_x() const { return m_inc_x; }
	int inc_y() const { return m_inc_y; }

	// Set the starting point of the glyph
	void start_point(double x, double y)
	{
		m_start_x = x;
		m_start_y = y;
	}

	void rewind(size_t)
	{
		m_cur_vertex = m_vertices;
		m_cur_flag = m_flags;
	}
	


	size_t vertex(double* x, double* y)
	{
		*x = m_start_x + *m_cur_vertex++;
		*y = m_start_y + *m_cur_vertex++;
		return *m_cur_flag++;
	}


};

struct GFont
{
	static constexpr size_t buf_size = 16384-32;


	GFont();
	//GFont(HDC dc, HFONT font);
	~GFont();
	
	// Set font and flip_y flag
	void initFont(HDC dc, HFONT font);
	void flip_y(bool flip) { m_flip_y = flip; }

	// parse the glyph information
	GGlyph& getGlyph(unsigned chr);

	// Return various values from the GLYPHMETRICS
	// once glyph() has been called.
	int origin_x() const { return m_origin_x; }
	int origin_y() const { return m_origin_y; }
	size_t width() const { return m_width; }
	size_t height() const { return m_height; }

	template <typename CharT>
	void measureText(double& w, double& h, const CharT* str);

protected:
	bool initGlyph(GGlyph& g, unsigned chr, bool hinted = true);

private:
	HDC			m_dc;
	HFONT		m_font;
	OUTLINETEXTMETRICW fOtm;
	std::map<unsigned, GGlyph > fGlyphs;

	char *		m_gbuf;
	const uint8_t* m_cur_flag;
	const double* m_cur_vertex;
	MAT2		m_mat2;

	int			m_origin_x;
	int			m_origin_y;
	size_t		m_width;
	size_t		m_height;

	bool		m_flip_y;
};


GFont::GFont()
	: m_dc(nullptr)
	, m_font(nullptr)
	, m_gbuf(new char[buf_size])
	, m_flip_y(false)
{
	// Start with identity matrix
	std::memset(&m_mat2, 0, sizeof(m_mat2));
	m_mat2.eM11.value = 1.0;
	m_mat2.eM22.value = 1.0;
}


GFont::~GFont()
{
	delete[] m_gbuf;
}

void GFont::initFont(HDC dc, HFONT font)
{
	m_dc = dc;
	m_font = font;
	size_t cjCopy = sizeof(fOtm);

	::GetOutlineTextMetrics(dc, cjCopy, &fOtm);
}

template<class CharT>
void GFont::measureText(double& w, double& h, const CharT* str)
{
	h = height();

	while (*str)
	{
		GGlyph& gl = font.getGlyph(*str++);
		w += gl.inc_x();
	}
}

static INLINE double fx_to_dbl(const FIXED& p)
{
	return double(p.value) + double(p.fract) * (1.0 / 65536.0);
}

static INLINE FIXED dbl_to_fx(double d)
{
	int l = long(d * 65536.0);
	return *(FIXED*)&l;
}

INLINE GGlyph& GFont::getGlyph(unsigned chr)
{
	// if we don't find the glyph
	// construct and insert it
	if (!fGlyphs.contains(chr))
	{
		GGlyph gl;
		initGlyph(gl, chr, false);
		fGlyphs.insert({ chr, gl });
	}
	
	return fGlyphs[chr];
}

// Read in a single glyph
INLINE bool GFont::initGlyph(GGlyph &g, unsigned chr, bool hinted)
{
	if (m_font == nullptr)
		return false;

	int unhinted = hinted ? 0 : GGO_UNHINTED;
	GLYPHMETRICS gm;
	ptrdiff_t total_size = GetGlyphOutlineA(m_dc,
		chr,
		GGO_NATIVE | unhinted,
		&gm,
		buf_size,
		(void*)m_gbuf,
		&m_mat2);

	if (GDI_ERROR == total_size)
		return false;

	m_origin_x = gm.gmptGlyphOrigin.x;
	m_origin_y = gm.gmptGlyphOrigin.y;
	m_width = gm.gmBlackBoxX;
	m_height = gm.gmBlackBoxY;
	g.m_inc_x = gm.gmCellIncX;
	g.m_inc_y = gm.gmCellIncY;

	g.reset(total_size);

	const char* cur_glyph = m_gbuf;
	const char* end_glyph = m_gbuf + total_size;

double* vertex_ptr = g.m_vertices;
uint8_t* flag_ptr = g.m_flags;

while (cur_glyph < end_glyph)
{
	const TTPOLYGONHEADER* th = (TTPOLYGONHEADER*)cur_glyph;

	const char* end_poly = cur_glyph + th->cb;
	const char* cur_poly = cur_glyph + sizeof(TTPOLYGONHEADER);

	*vertex_ptr++ = fx_to_dbl(th->pfxStart.x);
	*vertex_ptr++ = m_flip_y ?
		-fx_to_dbl(th->pfxStart.y) :
		fx_to_dbl(th->pfxStart.y);
	*flag_ptr++ = agg::path_cmd_move_to;

	while (cur_poly < end_poly)
	{
		const TTPOLYCURVE* pc = (const TTPOLYCURVE*)cur_poly;

		if (pc->wType == TT_PRIM_LINE)
		{
			int i;
			for (i = 0; i < pc->cpfx; i++)
			{
				*vertex_ptr++ = fx_to_dbl(pc->apfx[i].x);
				*vertex_ptr++ = m_flip_y ?
					-fx_to_dbl(pc->apfx[i].y) :
					fx_to_dbl(pc->apfx[i].y);
				*flag_ptr++ = agg::path_cmd_line_to;

			}
		}

		if (pc->wType == TT_PRIM_QSPLINE)
		{
			int u;
			// walk through points in spline
			for (u = 0; u < pc->cpfx - 1; u++)
			{
				POINTFX pnt_b = pc->apfx[u];
				POINTFX pnt_c = pc->apfx[u + 1];

				if (u < pc->cpfx - 2)
				{
					*(int*)&pnt_c.x = (*(int*)&pnt_b.x + *(int*)&pnt_c.x) / 2;
					*(int*)&pnt_c.y = (*(int*)&pnt_b.y + *(int*)&pnt_c.y) / 2;
				}

				*vertex_ptr++ = fx_to_dbl(pnt_b.x);
				*vertex_ptr++ = m_flip_y ?
					-fx_to_dbl(pnt_b.y) :
					fx_to_dbl(pnt_b.y);
				*flag_ptr++ = agg::path_cmd_curve3;

				*vertex_ptr++ = fx_to_dbl(pnt_c.x);
				*vertex_ptr++ = m_flip_y ?
					-fx_to_dbl(pnt_c.y) :
					fx_to_dbl(pnt_c.y);
				*flag_ptr++ = agg::path_cmd_curve3;
			}
		}
		cur_poly += sizeof(WORD) * 2 + sizeof(POINTFX) * pc->cpfx;
	}
	cur_glyph += th->cb;
	*vertex_ptr++ = 0.0;
	*vertex_ptr++ = 0.0;
	*flag_ptr++ = agg::path_cmd_end_poly | agg::path_flags_close | agg::path_flags_ccw;
}
*vertex_ptr++ = 0.0;
*vertex_ptr++ = 0.0;
*flag_ptr++ = agg::path_cmd_stop;

return true;
}


struct GFontFace
{
	HFONT fFontHandle;
	HDC		fDC;

	GFontFace();
	GFontFace(HFONT handle);
	GFontFace(const TCHAR* typeface, int fontHeight, bool bold, bool italic);

	~GFontFace();

	bool isValid();
	bool getFont(GFont& font, bool filpY = true);
	bool initFromName(const TCHAR* typeface, int fontHeight, bool bold, bool italic);
};

GFontFace::GFontFace() :fFontHandle(nullptr), fDC(nullptr) {}

GFontFace::GFontFace(HFONT handle) : fFontHandle(handle), fDC(nullptr) {}

GFontFace::GFontFace(const TCHAR* typeface, int fontHeight, bool bold, bool italic)
	: fFontHandle(nullptr), fDC(nullptr)
{
	initFromName(typeface, fontHeight, bold, italic);
}

GFontFace::~GFontFace()
{
	if (nullptr != fFontHandle)
		::DeleteObject(fFontHandle);

	ReleaseDC(nullptr, fDC);
}

bool GFontFace::isValid()
{
	return nullptr != fFontHandle;
}

// Get an instance of a font for the DeviceContext
bool GFontFace::getFont(GFont & font, bool flipY)
{
	font.initFont(fDC, fFontHandle);
	font.flip_y(flipY);

	return true;
}

bool GFontFace::initFromName(const TCHAR* typeface, int fontHeight, bool bold, bool italic)
{
	// Font parameters
	int fontWidth = 0;
	int iAngle = 0;

	fFontHandle = ::CreateFont(fontHeight,
		fontWidth,
		iAngle,
		iAngle,
		bold ? 700 : 400,
		italic,
		FALSE,
		FALSE,
		ANSI_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		ANTIALIASED_QUALITY,
		FF_DONTCARE,
		typeface);

	if (nullptr == fFontHandle)
		return false;

	fDC = ::GetDC(nullptr);
	HGDIOBJ old_font = ::SelectObject(fDC, fFontHandle);

	return true;
}

// Create an instance of a Font based on typeface and height
//GFontFace GFontFace::create(const TCHAR* typeface, int fontHeight, bool bold, bool italic)
//{
//	return GFontFace(typeface, fontHeight, bold, italic);
//}