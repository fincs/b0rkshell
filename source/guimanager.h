#pragma once
#include "common.h"

class CGuiManager : public IGuiManager
{
public:

	inline int GetInterfaceVersion() { return COOPGUI_GUIMANAGER_VER; }

	void RunApplication(IApplication* pApp);
	IApplication* GetActiveApp() { return g_curApp->GetRawPtr(); }

	IFont* LoadFont(const char* face, int size);
	inline IFont* GetSystemFont()
	{
		extern CFont font;
		return &font;
	}

	inline bool LoadGrf(GrfFile& grf, FILE* f)
	{
		return ((CGrf*)&grf)->Load(f);
	}

	void SwitchTo(AppWrapper* pApp);
	inline void SwitchTo(IApplication* pApp)
	{
		AppWrapper* app = nullptr;
		if (pApp) app = (AppWrapper*) pApp->GetOrSetCookie(nullptr);
		SwitchTo(app);
	}
};

extern CGuiManager g_guiManager;

#define EnableGuiMon()  __setguimon(&g_guiManager)
#define DisableGuiMon() __setguimon(nullptr)
