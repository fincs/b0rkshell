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

class CTouchPos : public touchPosition
{
public:
	CTouchPos()
	{
		touchRead(this);
	}
	bool inRegion(int x, int y, int w, int h)
	{
		int x2 = x + w;
		int y2 = y + h;
		return (px >= x && px < x2) && (py >= y && py < y2);
	}
};
