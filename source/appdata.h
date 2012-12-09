#pragma once
#include "common.h"

struct CProvideFileIcon : public IFileIcon
{
	int refCount;
	CGrf* data;

	CProvideFileIcon() : refCount(0), data(nullptr)
	{
	}

	bool isLoaded()
	{
		return data != nullptr;
	}

	IFileIcon* getSelf()
	{
		refCount ++;
		return this;
	}
	
	void Dispose()
	{
		if (!--refCount)
		{
			delete data;
			data = nullptr;
		}
	}

	const color_t* GetData()
	{
		return data ? (const color_t*) data->gfxData : nullptr;
	}
};

class CStaticFileIcon : public IFileIcon
{
	const CGrf& data;

public:
	CStaticFileIcon(const CGrf& grf) : data(grf) { }
	void Dispose() { }
	const color_t* GetData() { return (const color_t*) data.gfxData; }

	IFileIcon* getSelf() { return this; }
};

class CAppData
{
	bool bLoaded;
	CManifest info;
	CGrf *cachedIcon;
	char* name;
	CProvideFileIcon cachedFileIcon;

public:
	inline CAppData() : bLoaded(false), cachedIcon(nullptr), name(nullptr) { }
	inline ~CAppData() { Free(); }

	CGrf* GetIcon();
	IFileIcon* GetFileIcon();

	inline const char* GetName()
	{
		return name;
	}

	inline bool Load(const char* manifest, const char* appName)
	{
		if ((bLoaded = info.Load(manifest)))
		{
			name = strdup(appName);
			strchr(name, '.')[0] = 0;
		}
		return bLoaded;
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

	inline void FreeName()
	{
		if (name)
		{
			free(name);
			name = nullptr;
		}
	}

	inline void Free()
	{
		info.Free();
		FreeIcon();
		FreeName();
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

	inline const char* GetFileIconPath()
	{
		return info.Read("filetype-icon");
	}

	inline const char* GetFileCommand()
	{
		return info.Read("filetype-command");
	}

	void Run(const char* parameter = nullptr);
};
