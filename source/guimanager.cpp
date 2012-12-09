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

static const char* extractExt(const char* aFileName)
{
	const char* ext = aFileName + strlen(aFileName) - 1;
	bool foundIt = false;
	for (; ext >= aFileName && !((foundIt = *ext == '.') || *ext == '/'); ext --);
	return foundIt ? ext+1 : nullptr;
}

static inline CFileType* getFT(const char* aFileName, bool& isMulti, const char** pExt = nullptr)
{
	const char* ext = extractExt(aFileName);
	if (!ext) return nullptr;
	if (pExt) *pExt = ext;
	return FindFileType(ext, isMulti);
}

void CGuiManager::OpenFile(const char* aFileName)
{
	bool isMulti;
	CFileType* ft = getFT(aFileName, isMulti);

	if (!ft)
		return;

	CAppData* app = ft->opensWith;

	if (isMulti)
	{
		// TODO: app selector :D
		app = nullptr;
	}

	printf("Opens with %p (isMulti=%d)\n", app, isMulti);

	if (!app)
		return;

	app->Run(aFileName);
}

static CStaticFileIcon oFiGeneric(fiGenericFile);
static CStaticFileIcon oFiConflict(fiConflictFile);

IFileIcon* CGuiManager::GetFileIcon(const char* aFileName)
{
	bool isMulti;
	CFileType* ft = getFT(aFileName, isMulti);

	if (!ft)
		return oFiGeneric.getSelf();

	if (isMulti)
		return oFiConflict.getSelf();

	IFileIcon* ico = ft->opensWith->GetFileIcon();
	return ico ? ico : oFiGeneric.getSelf();
}

void CGuiManager::GetFileDescription(const char* aFileName, char* buffer, size_t size)
{
	bool isMulti;
	const char* ext = nullptr;
	CFileType* ft = getFT(aFileName, isMulti, &ext);

	if (!ft || isMulti)
		snprintf(buffer, size, ".%s file", ext);
	else
		strncpy(buffer, ft->description, size);
}
