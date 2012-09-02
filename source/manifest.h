#pragma once
#include "common.h"

typedef struct
{
	int keyCount;
	union
	{
		struct
		{
			int keyPos;
			int valPos;
		} keys[0];
		char buf[0];
	};
} metadata_t;

class CManifest
{
	metadata_t* pData;
	char* Find(const char* key, int min, int max);
public:
	inline CManifest() : pData(nullptr) { }
	inline ~CManifest() { Free(); }
	bool Load(const char* filename);
	void Free();
	inline char* Read(const char* key)
	{
		return Find(key, 0, pData->keyCount - 1);
	}
};
