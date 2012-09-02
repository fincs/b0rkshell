#include "common.h"

bool CManifest::Load(const char* filename)
{
	Free();
	FILE* f = fopen(filename, "rb");
	if (!f) return false;
	fseek(f, 0, SEEK_END);
	int size = ftell(f);
	rewind(f);
	void* mem = malloc(size);
	if (!mem)
	{
		fclose(f);
		return false;
	}
	fread(mem, 1, size, f);
	fclose(f);
	pData = (metadata_t*) mem;
	return true;
}

void CManifest::Free()
{
	if (pData)
	{
		free(pData);
		pData = nullptr;
	}
}

char* CManifest::Find(const char* key, int min, int max)
{
	if (!pData) return nullptr;

	while (max >= min)
	{
		int mid = (min + max) / 2;
		int rc = strcmp(pData->buf + pData->keys[mid].keyPos, key);
		if (rc < 0)
			min = mid + 1;
		else if (rc > 0)
			max = mid - 1;
		else
			return pData->buf + pData->keys[mid].valPos;
	}
	return nullptr;
}
