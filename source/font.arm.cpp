#include "common.h"

bool CFont::loadFont(const char* filename)
{
	if (f)
	{
		fclose(f); f = NULL;
		foreach(pageMapIter, i, pageMap)
			free(i->second);
		pageMap.clear();
	}
	f = fopen(filename, "rb");
	if (!f) return false;
	ffnt_header_t hdr;
	fread(&hdr, sizeof(ffnt_header_t), 1, f);
	// TODO: check magic & version
	npages = hdr.npages;
	height = hdr.height;
	baseline = hdr.baseline;
	loadPage(0);
	return true;
}

CFont::CFont() : f(0), cachedPageN(-1)
{
}

CFont::~CFont()
{
	if (f) fclose(f);
	foreach(pageMapIter, i, pageMap)
		free(i->second);
}

ffnt_page_t* CFont::getPage(int page)
{
	pageMapIter it = pageMap.find(page);
	if (it != pageMap.end())
		return it->second;
	else
		return loadPage(page);
}

ffnt_page_t* CFont::loadPage(int page)
{
	if (npages < page) return NULL;

	fseek(f, sizeof(ffnt_header_t) + page*sizeof(ffnt_pageentry_t), SEEK_SET);

	ffnt_pageentry_t ent;
	fread(&ent, sizeof(ffnt_pageentry_t), 1, f);

	if (!ent.size) return NULL;

	ffnt_page_t* pageData = (ffnt_page_t*) malloc(ent.size);
	if (!pageData) return NULL;

	fseek(f, ent.offset, SEEK_SET);
	fread(pageData, 1, ent.size, f);

	pageMap.insert(ustl::make_pair(page, pageData));
	return pageData;
}

//-----------------------------------------------------------------------------

bool CFont::Load(const char* face, int size)
{
	static char buf[PATH_MAX];
	snprintf(buf, sizeof(buf), GUI_FONTS_DIR "/%s%d.ffnt", face, size);
	return loadFont(buf);
}

bool CFont::LoadGlyph(int codePoint, glyph_t* /*out*/ pGlyph)
{
	int pageN = codePoint >> 8;
	int entry = codePoint & 0xFF;

	ffnt_page_t* page;
	if (cachedPageN == pageN)
		page = cachedPage;
	else
	{
		ffnt_page_t* page2 = getPage(pageN);
		if (!page2) return false;
		cachedPageN = pageN;
		cachedPage = page2;
		page = page2;
	}

	int off = page->hdr.pos[entry];
	if (off == 0xFFFF) return false;
	pGlyph->width = page->hdr.widths[entry];
	pGlyph->height = page->hdr.heights[entry];
	pGlyph->advance = page->hdr.advances[entry];
	pGlyph->posX = page->hdr.posX[entry];
	pGlyph->posY = page->hdr.posY[entry];
	pGlyph->data = page->data + off;
	return true;
}

//---------------------------------------------------------------------------

static void DrawGlyph(color_t* buf, const byte_t* data, const rect_t* pRect, color_t brush, int stride)
{
	int x0 = pRect->x, y0 = pRect->y, x1 = x0 + pRect->w, y1 = y0 + pRect->h;

	for (int y = y0; y < y1; y ++)
	{
		// Draw a row
		for (int x = x0; x < x1; x += 8)
		{
			int d = *data++; int pos = x + y * stride; int q = 1 << 8;
			if (d & (q>>=1)) buf[pos]   = brush;
			if (d & (q>>=1)) buf[pos+1] = brush;
			if (d & (q>>=1)) buf[pos+2] = brush;
			if (d & (q>>=1)) buf[pos+3] = brush;
			if (d & (q>>=1)) buf[pos+4] = brush;
			if (d & (q>>=1)) buf[pos+5] = brush;
			if (d & (q>>=1)) buf[pos+6] = brush;
			if (d & (q>>=1)) buf[pos+7] = brush;
		}
	}
}

int CFont::PrintText(const surface_t* s, int x, int y, const char* text, color_t brush, word_t flags)
{
	typedef ustl::utf8in_iterator<const char*> utf8_iterator;

	color_t* buf = s->buffer;

	if (!(flags & PrintTextFlags::AtBaseline))
		y += GetBaseline();

	int w = s->width, h = s->height, stride = s->stride;
	int fontH = GetHeight();

	const char* end_pos = text+strlen(text);
	utf8_iterator end(end_pos);

	for (utf8_iterator i(text); i < end; ++ i)
	{
		int c = *i;

		if (c == '\n')
		{
			y += fontH;
			x = 0;
			continue;
		}

		glyph_t glyph;

		if (!LoadGlyph(c, &glyph))
			continue;

		if ((w - x) < glyph.advance)
		{
			y += fontH;
			x = 0;
		}

		int leftH = y + glyph.posY + glyph.height;
		if (leftH >= h)
			break;

		rect_t r = { x + glyph.posX, y + glyph.posY, glyph.width, glyph.height };
		DrawGlyph(buf, glyph.data, &r, brush, stride);
		x += glyph.advance;
	}

	return 0;
}
