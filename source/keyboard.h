#pragma once
#include "common.h"

class CKeyboard : public IKeyboard
{
	bool bShifted, bCapsLock;
	color_t* bmpBuf;
	char* readBuf;
	ssize_t readBufPos;
	size_t readBufLen;

	int getKey(const TouchPos& pos);

public:
	NON_THROWING_ALLOCATION;

	inline void Dispose() { delete this; }
	inline int GetInterfaceVersion() { return 1; }

	inline CKeyboard() : bShifted(false), bCapsLock(false), bmpBuf(nullptr), readBuf(nullptr) { }
	~CKeyboard();

	void renderLabels();

	void SetDrawBuffer(color_t* bmpBuf);
	const GrfFile* GetGraphics();
	int OnClick(const TouchPos& pos);
	ssize_t SyncRead(char* buf, size_t len);
	inline bool InSyncRead() { return readBuf != nullptr; }
};

IKeyboard* newKeyboard(const char* layout);
