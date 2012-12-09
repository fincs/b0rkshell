#pragma once
#include "common.h"

struct CFileType
{
	CFileType** metaPtr;
	union
	{
		char ext[8]; // up to 8 chars
		u64 extId;
	};
	CAppData* opensWith;
	char* description;
	CFileType *prev, *next;

	inline CFileType() : metaPtr(nullptr), extId(0), opensWith(nullptr), description(nullptr), prev(nullptr), next(nullptr)
	{
	}

	inline ~CFileType()
	{
		if (description)
			free(description);
	}

	inline void Chain(CFileType* q)
	{
		prev = q;
		q->next = this;
	}
};
