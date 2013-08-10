#pragma once
#include "common.h"

class AppWrapper
{
	IApplication* app;
	thread_t thr;
	const AppInfo* info;

public:
	inline AppWrapper(IApplication* pApp) : app(pApp)
	{
		info = pApp->GetInfo();
		thr = ThrGetSelf();
		pApp->GetOrSetCookie((AppCookie)this);
	}

	inline bool IsAlive() { return !(info->Flags & AppFlags::Closed); }
	inline IApplication* GetRawPtr() { return app; }
	inline const AppInfo* GetInfo() { return info; }

	static int _thunk_OnActivate(void* prm) { ((IApplication*)prm)->OnActivate(); return 0; }
	static int _thunk_OnDeactivate(void* prm) { ((IApplication*)prm)->OnDeactivate(); return 0; }
	static int _thunk_OnVBlank(void* prm) { ((IApplication*)prm)->OnVBlank(); return 0; }
	static int _thunk_OnBgProcess(void* prm) { ((IApplication*)prm)->OnBgProcess(); return 0; }

	inline void OnActivate() { ThrRunInContext(thr, _thunk_OnActivate, app); }
	inline void OnDeactivate() { ThrRunInContext(thr, _thunk_OnDeactivate, app); }
	inline void OnVBlank() { ThrRunInContext(thr, _thunk_OnVBlank, app); }
	inline void OnBgProcess() { ThrRunInContext(thr, _thunk_OnBgProcess, app); }

	NON_THROWING_ALLOCATION;
};

class AppList
{
	struct AppNode
	{
		AppNode* next;
		AppNode* prev;
		AppWrapper* app;

		NON_THROWING_ALLOCATION;
	};

	AppNode root;
	int appCount;

public:
	typedef struct { } *Cookie;

	inline AppList() : appCount(0)
	{
		root.next = &root;
		root.prev = &root;
		root.app = nullptr;
	}

	inline int GetCount() { return appCount; }

	inline Cookie AddApp(AppWrapper* pApp)
	{
		if (appCount == MAX_RUNNING_APP_COUNT)
			return nullptr;

		AppNode* pNode = new AppNode;
		if (!pNode) return nullptr;
		pNode->app = pApp;
		pNode->prev = root.prev;
		pNode->next = &root;
		root.prev->next = pNode;
		root.prev = pNode;
		appCount ++;
		return (Cookie)pNode;
	}

	inline void RemoveApp(Cookie cookie)
	{
		AppNode* pNode = (AppNode*) cookie;
		pNode->prev->next = pNode->next;
		pNode->next->prev = pNode->prev;
		delete pNode;
		appCount --;
	}

	inline void RemoveApp(AppWrapper* pApp)
	{
		for (AppNode* pNode = root.next; pNode != &root; pNode = pNode->next)
			if (pNode->app == pApp)
			{
				RemoveApp((Cookie) pNode);
				break;
			}
	}

	template <typename LambdaType>
	inline void Iterate(LambdaType lambda)
	{
		for (AppNode* pNode = root.next; pNode != &root;)
		{
			AppNode* pNext = pNode->next;
			if (!lambda(pNode->app))
				break;
			pNode = pNext;
		}
	}

	inline ~AppList()
	{
		for (AppNode* pNode = root.next; pNode != &root;)
		{
			AppNode* pNext = pNode->next;
			delete pNode;
			pNode = pNext;
		}
	}
};

extern AppList g_appList;
extern AppWrapper* g_curApp;
