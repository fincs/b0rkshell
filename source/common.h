#pragma once
#include <coopgui.h>

using namespace FeOS::UI;

// Internal coopgui function
extern "C" void __setguimon(IGuiManager* newMon);

#define NON_THROWING_ALLOCATION \
	inline void* operator new (size_t sz) throw() { return malloc(sz); } \
	inline void operator delete (void* data) throw() { free(data); }

typedef struct
{
	int x, y, w, h;
} rect_t;

#define MAX_RUNNING_APP_COUNT 8
#define PAGE_COUNT 12
#define APPS_PER_PAGE 6
#define APP_COUNT (PAGE_COUNT*APPS_PER_PAGE)
#define GUI_DIRECTORY "/data/FeOS/gui"
#define GUI_ASSET_DIR GUI_DIRECTORY "/assets"
#define GUI_FONTS_DIR GUI_DIRECTORY "/fonts"
#define GUI_FTYPE_DIR GUI_DIRECTORY "/ftypes"

#include "grf.h"
#include "font.h"
#include "manifest.h"
#include "appdata.h"
#include "applist.h"
#include "filetypes.h"
#include "keyboard.h"

#include "guimanager.h"

extern CAppData g_appData[APP_COUNT];
extern int g_appCount;
extern bool g_appListChanged;

extern CFileType** g_fileTypes;
extern CFileType* g_firstFileType;
extern int g_filetypeCount;
extern bool bRunningDMApp;

extern CGrf fiGenericFile, fiConflictFile;

void LoadApps();
void LoadFileTypes();

CFileType* FindFileType(const char* ext, bool& isMulti);

void videoReset();
void videoInit();

