#include "common.h"

CGuiManager g_guiManager;

IFont* CGuiManager::LoadFont(const char* face, int size)
{
	CHeapFont* pFont = new CHeapFont;
	if (!pFont) return nullptr;
	if (!pFont->Load(face, size))
	{
		pFont->Dispose();
		return nullptr;
	}
	return pFont;
}

void CGuiManager::RunApplication(IApplication* pApp)
{
	AppWrapper wrap(pApp);

	auto cookie = g_appList.AddApp(&wrap);
	if (!cookie)
		return;

	g_appListChanged = true;

	if (g_curApp)
		g_curApp->OnDeactivate();

	swiWaitForVBlank();
	videoReset();
	g_curApp = &wrap;
	pApp->OnActivate();

	while (wrap.IsAlive())
		swiWaitForVBlank();

	if (g_curApp == &wrap)
	{
		pApp->OnDeactivate();
		g_curApp = nullptr;
	}

	g_appList.RemoveApp(cookie);
	g_appListChanged = true;

	if (!g_curApp)
	{
		swiWaitForVBlank();
		videoReset();
		videoInit();
	}
}

void CGuiManager::SwitchTo(AppWrapper* pApp)
{
	if (g_curApp)
		g_curApp->OnDeactivate();

	swiWaitForVBlank();
	videoReset();
	g_curApp = pApp;
	if (g_curApp)
		g_curApp->OnActivate();
	else
		videoInit();
}
