#pragma once
#include "common.h"

#define CHUNK_ID(a,b,c,d) ((word_t)( (a) | (b)<<8 | (c)<<16 | (d)<<24 ))
#define ID_RIFF CHUNK_ID('R','I','F','F')
#define ID_GRF  CHUNK_ID('G','R','F',' ')
#define ID_GRF_HDR  CHUNK_ID('H','D','R',' ')
#define ID_GRF_GFX  CHUNK_ID('G','F','X',' ')
#define ID_GRF_MAP  CHUNK_ID('M','A','P',' ')
#define ID_GRF_MTIL CHUNK_ID('M','T','I','L')
#define ID_GRF_MMAP CHUNK_ID('M','M','A','P')
#define ID_GRF_PAL  CHUNK_ID('P','A','L',' ')

struct CGrf : public GrfFile
{
	bool Load(FILE* f);
	inline bool Load(const char* fileName)
	{
		FILE* f = fopen(fileName, "rb");
		if (!f) return nullptr;

		bool rc = Load(f);
		fclose(f);
		return rc;
	}
};
