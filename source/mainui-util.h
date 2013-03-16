#pragma once
#include "common.h"

class Bumper
{
	u16 hlColor, nonhlColor;
	vu16* bumpState;
	int nBumps;

public:
	void init(vu16* pal, int _nBumps = PAGE_COUNT)
	{
		hlColor = pal[23];
		nonhlColor = pal[9];
		bumpState = pal + 0x20;
		nBumps = _nBumps;
	}

	void SetBumpState(int id, bool state)
	{
		bumpState[id] = state ? hlColor : nonhlColor;
	}

	void SelectBump(int id)
	{
		for (int i = 0; i < nBumps; i ++)
			SetBumpState(i, i == id);
	}

	int GetBumpCount() { return nBumps; }
};
