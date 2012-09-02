#include "common.h"
#include <decompress.h>

static inline word_t read32(FILE* f)
{
	word_t v;
	fread(&v, sizeof(word_t), 1, f);
	return v;
}

static inline bool decompress(memchunk_t* pChunk, FILE* f, size_t compSize)
{
	word_t compType = read32(f);
	word_t decompSize = compType >> 8;
	compType &= 0xF0;

	// Alloc first the uncompressed buffer in order to avoid heap fragmentation
	memchunk_t chunk = MemChunk_Alloc(decompSize);
	if (!chunk) return false;

	if (compType == 0) // Shortcut for uncompressed
	{
		if (decompSize != (compSize - 4))
		{
			MemChunk_Free(chunk);
			return false;
		}
		fread(chunk, 1, decompSize, f);
		DC_FlushRange(chunk, decompSize);
		*pChunk = chunk;
		return true;
	}

	void* compBuf = malloc(compSize);
	if (!compBuf)
	{
		MemChunk_Free(chunk);
		return false;
	}

	bool bSuccess = true;
	fseek(f, -4, SEEK_CUR);
	fread(compBuf, 1, compSize, f);

	decompressAuto(compBuf, chunk, false);

	free(compBuf);
	if (bSuccess)
	{
		DC_FlushRange(chunk, decompSize);
		*pChunk = chunk;
		return true;
	}

	MemChunk_Free(chunk);
	return false;
}

bool CGrf::Load(FILE* f)
{
	if (read32(f) != ID_RIFF) return false;
	int fileSize = read32(f) + 8;
	if (read32(f) != ID_GRF) return false;

	bool bReadHeader = false;
	while (ftell(f) < fileSize)
	{
		word_t id = read32(f);
		word_t size = read32(f);

		if (!bReadHeader)
		{
			if (id != ID_GRF_HDR) return false;
			if (size != sizeof(GrfHeader)) return false;
			fread(&hdr, size, 1, f);
			bReadHeader = true;
			continue;
		}

		// Data chunk - figure out where to write
		memchunk_t* pChunk = nullptr;
		switch (id)
		{
			case ID_GRF_GFX:  pChunk = &gfxData;  break;
			case ID_GRF_MAP:  pChunk = &mapData;  break;
			case ID_GRF_MTIL: pChunk = &mtilData; break;
			case ID_GRF_MMAP: pChunk = &mmapData; break;
			case ID_GRF_PAL:  pChunk = &palData;  break;
		}

		if (!pChunk)
		{
			fseek(f, size, SEEK_CUR);
			continue;
		}

		if (!decompress(pChunk, f, size))
			return false;
	}
	return true;
}
