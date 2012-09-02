#pragma once
#include "common.h"

class CAppData
{
	bool bLoaded;
	CManifest info;
	CGrf* cachedIcon;

public:
	inline CAppData() : bLoaded(false), cachedIcon(nullptr) { }
	inline ~CAppData() { FreeIcon(); }

	CGrf* GetIcon();

	inline bool Load(const char* manifest)
	{
		return (bLoaded = info.Load(manifest));
	}

	inline bool IsLoaded()
	{
		return bLoaded;
	}

	inline void FreeIcon()
	{
		if (cachedIcon)
		{
			delete cachedIcon;
			cachedIcon = nullptr;
		}
	}

	inline void Free()
	{
		info.Free();
		FreeIcon();
		bLoaded = false;
	}

	inline const char* GetTitle()
	{
		return info.Read("title");
	}

	inline const char* GetDescription()
	{
		return info.Read("description");
	}

	inline const char* GetIconPath()
	{
		return info.Read("icon");
	}

	inline const char* GetCommand()
	{
		return info.Read("command");
	}

	inline bool IsDirectMode()
	{
		return info.Read("direct-mode-app") != nullptr;
	}
};
