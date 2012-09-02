#pragma once
#include "common.h"

typedef struct
{
	char magic[4]; // 'fFNT'
	int version; // 0
	hword_t npages;
	byte_t height;
	byte_t baseline;
} ffnt_header_t;

typedef struct
{
	size_t size, offset;
} ffnt_pageentry_t;

typedef struct
{
	hword_t pos[0x100];
	byte_t widths[0x100];
	byte_t heights[0x100];
	char_t advances[0x100];
	char_t posX[0x100];
	char_t posY[0x100];
} ffnt_pagehdr_t;

typedef struct
{
	ffnt_pagehdr_t hdr;
	byte_t data[1];
} ffnt_page_t;

class CFont : public IFont
{
	typedef ustl::map<int,ffnt_page_t*> pageMapType;
	typedef pageMapType::iterator pageMapIter;
	FILE* f;
	pageMapType pageMap;

	int npages, height, baseline;

	int cachedPageN;
	ffnt_page_t* cachedPage;

	ffnt_page_t* loadPage(int page);
	bool loadFont(const char* name);
	ffnt_page_t* getPage(int page);

public:

	inline void Dispose() { }

	CFont();
	~CFont();

	bool Load(const char* face, int size);
	inline int GetHeight() { return height; }
	inline int GetBaseline() { return baseline; }
	bool LoadGlyph(int codePoint, glyph_t* /*out*/ pGlyph);

	int PrintText(color_t* buf, int x, int y, const char* text, color_t brush = Colors::Black, word_t flags = 0);
};

class CHeapFont : public CFont
{
public:
	NON_THROWING_ALLOCATION;

	inline void Dispose()
	{
		delete this;
	}
};
