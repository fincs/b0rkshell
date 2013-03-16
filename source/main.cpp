#include "common.h"
#include "mainui-util.h"
#include <time.h>

CGrf background, dummy, defappicon, selection, topscr, fiGenericFile, fiConflictFile;
CFont font;
Bumper bump;
int page = 0;
int selectedApp = 0;
SpriteEntry* oamSubMem;
u16* bmpBuf;

CAppData g_appData[APP_COUNT];
int g_appCount;

CFileType** g_fileTypes;
CFileType* g_firstFileType;
int g_filetypeCount;

AppList g_appList;
AppWrapper* g_curApp;

bool g_appListChanged;

void clearBitmap()
{
	dmaFillHalfWords(Colors::Transparent, bmpBuf, 256*192*2);
}

void clearBmpLines(int start, int nlines)
{
	dmaFillHalfWords(Colors::Transparent, bmpBuf + start*256, nlines*256*2);
}

static int myPrintText(int x, int y, const char* text, color_t brush = Colors::Black, word_t flags = PrintTextFlags::AtBaseline)
{
	return font.PrintText(bmpBuf, x, y, text, brush, flags);
}

void videoReset()
{
	FeOS_VideoReset();

	FeOS_SetAutoUpdate(AUTOUPD_OAM, true);
	FeOS_SetAutoUpdate(AUTOUPD_BG, true);
	FeOS_SetAutoUpdate(AUTOUPD_KEYS, true);
}

static void updCursor()
{
	CAppData& app = g_appData[selectedApp];

	int slPage = selectedApp / 6;
	if (slPage != page)
		oamSubMem[6].isHidden = true;
	else
	{
		int x = selectedApp % 6;
		int xPos = 16 + (x % 3) * (64+16) - 8;
		int yPos = 48 + (x / 3) * (64+8) + 64 - 8;
		oamSubMem[6].x = xPos;
		oamSubMem[6].y = yPos;
		oamSubMem[6].isHidden = false;
	}

	clearBmpLines(128, 192-128);
	myPrintText(24, 151, app.GetTitle());
	myPrintText(24, 151+16, app.GetDescription());
}

static void unloadPageIcons(int page)
{
	for (int i = 0; i < 6; i ++)
		g_appData[page*3 + i].FreeIcon();
}

static void loadPageIcons()
{
	memchunk_t gfxPtrs[6];
	for (int i = 0; i < 6; i ++)
	{
		CGrf* grf = g_appData[page*6 + i].GetIcon();
		gfxPtrs[i] = grf ? grf->gfxData : nullptr;
	}

	u16* gfx = SPRITE_GFX_SUB;
	for (int i = 0; i < 6; i ++)
	{
		if (gfxPtrs[i])
		{
			dmaCopy(gfxPtrs[i], gfx, MemChunk_GetSize(gfxPtrs[i]));
			oamSubMem[i].isHidden = false;
		}else
			oamSubMem[i].isHidden = true;
		gfx += 64*64;
	}
}

static void updAppList()
{
	if (!g_appListChanged) return;
	g_appListChanged = false;

	int i;

	for (i = 0; i < MAX_RUNNING_APP_COUNT; i ++)
		oamSubMem[7+i].isHidden = true;

	i = 7;
	u16* gfx = SPRITE_GFX_SUB + 6*64*64 + 16*16;
	g_appList.Iterate([&] (AppWrapper* app)
	{
		color_t* pAppIcon = app->GetInfo()->Icon;
		dmaCopy(pAppIcon ? pAppIcon : defappicon.gfxData, gfx, 16*16*2);
		oamSubMem[i++].isHidden = false;
		gfx += 16*16;
		return true;
	});
}

void videoInit()
{
	videoSetModeSub(MODE_0_2D);
	oamInit(&oamSub, SpriteMapping_Bmp_1D_128, false);
	oamSubMem = FeOS_GetOAMMemory(&oamSub);

	int bgId = bgInitSub(3, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	bgSetPriority(bgId, 3);
	dmaCopy(background.gfxData, bgGetGfxPtr(bgId), MemChunk_GetSize(background.gfxData));
	dmaCopy(background.mapData, bgGetMapPtr(bgId), MemChunk_GetSize(background.mapData));
	dmaCopy(background.palData, BG_PALETTE_SUB,    MemChunk_GetSize(background.palData));

	videoSetMode(MODE_3_2D);

	int bgBmp = bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 2, 0);
	bmpBuf = bgGetGfxPtr(bgBmp);
	clearBitmap();

	int bgTop = bgInit(2, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	bgSetPriority(bgTop, 3);
	dmaCopy(topscr.gfxData, bgGetGfxPtr(bgTop), MemChunk_GetSize(topscr.gfxData));
	dmaCopy(topscr.mapData, bgGetMapPtr(bgTop), MemChunk_GetSize(topscr.mapData));
	dmaCopy(topscr.palData, BG_PALETTE,         MemChunk_GetSize(topscr.palData));

	u16* gfx = SPRITE_GFX_SUB;
	for (int i = 0; i < 6; i ++)
	{
		int xPos = 16 + (i % 3) * (64+16);
		int yPos = 48 + (i / 3) * (64+8);
		oamSet(&oamSub, i, xPos, yPos, 1, 15, SpriteSize_64x64, SpriteColorFormat_Bmp, gfx, -1, 0, false, false, false, false);
		gfx += 64*64;
	}
	{
		dmaCopy(selection.gfxData, gfx, MemChunk_GetSize(selection.gfxData));
		oamSet(&oamSub, 6, 0, 0, 0, 15, SpriteSize_16x16, SpriteColorFormat_Bmp, gfx, -1, 0, false, false, false, false);
		gfx += 16*16;
	}
	for (int i = 0; i < MAX_RUNNING_APP_COUNT; i ++)
	{
		oamSet(&oamSub, 7+i, 64+(i*(16+8)), 16, 1, 15, SpriteSize_16x16, SpriteColorFormat_Bmp, gfx, -1, 0, false, false, false, false);
		oamSubMem[7+i].isHidden = true;
		gfx += 16*16;
	}

	g_appListChanged = true;

	bump.init(BG_PALETTE_SUB);
	bump.SelectBump(page);
	updCursor();
	loadPageIcons();
	updAppList();
}

int emulatedMode = MODE_CONSOLE;
bool bRunningDMApp = false;

static void doDirectMode()
{
	if (!bRunningDMApp)
	{
		fprintf(stderr, "Direct mode applications cannot\n");
		fprintf(stderr, "be run from coop GUI apps nor\n");
		fprintf(stderr, "from the emulated console.\n");
		exit(1);
	}
	if (emulatedMode == MODE_DIRECT)
		return;

	emulatedMode = MODE_DIRECT;

	swiWaitForVBlank();

	if (g_curApp)
		g_curApp->OnDeactivate();

	videoReset();
}

static void doConsoleMode()
{
	if (emulatedMode == MODE_CONSOLE)
		return;
	emulatedMode = MODE_CONSOLE;
	swiWaitForVBlank();
	videoReset();
	if (!g_curApp)
		videoInit();
	else
		g_curApp->OnActivate();
}

static int doGetMode()
{
	return emulatedMode;
}

static const modeshim_t modeShim = { doDirectMode, doConsoleMode, doGetMode };

static const char* getDayEnd(int d)
{
	static const char* dayEnds[] = { "th", "st", "nd", "rd", "th", "th", "th", "th", "th", "th" };
	div_t q = div(d, 10);
	return q.quot != 1 ? dayEnds[q.rem] : dayEnds[0];
}

static void renderTopLines()
{
	static const char* monthNames[] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
	static const char* dayNames[] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
	
	clearBmpLines(0, 128);
	char buf[128];

	time_t rawtime;
	time(&rawtime);
	struct tm* timeinfo = localtime(&rawtime);

	sprintf(buf, "%02d:%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
	myPrintText(16, 23, buf);

	sprintf(buf, "%s, %s %d%s %d", dayNames[timeinfo->tm_wday], monthNames[timeinfo->tm_mon]
		, timeinfo->tm_mday, getDayEnd(timeinfo->tm_mday), timeinfo->tm_year + 1900);
	myPrintText(16, 23+16, buf);

	/*
	for (int i = 2; i < 6; i ++)
	{
		sprintf(buf, "LINE%d", i+1);
		myPrintText(16, 23+i*16, buf);
	}
	*/
	usagestats_t memStats;
	FeOS_GetMemStats(&memStats);
	sprintf(buf, "%d KiB (%d%%) of free RAM", memStats.free / 1024, (memStats.free * 100) / memStats.total);
	myPrintText(16, 23+2*16, buf);
}

void executeApp()
{
	g_appData[selectedApp].Run();
}

static void SwitchToApp(int id)
{
	int i = 0;
	AppWrapper* pApp = nullptr;
	g_appList.Iterate([&] (AppWrapper* app)
	{
		if (i == id)
		{
			pApp = app;
			return false;
		}
		i ++;
		return true;
	});
	g_guiManager.SwitchTo(pApp);
}

static bool MainVBlank()
{
	renderTopLines();
	updAppList();

	word_t kDown = keysDown();
	CTouchPos pos;

	// Page moving code
	do
	{
		int disp = 0;
		if (kDown & KEY_LEFT) disp = -1;
		else if (kDown & KEY_RIGHT) disp = +1;
		else if (kDown & KEY_UP) disp = -6;
		else if (kDown & KEY_DOWN) disp = +6;
		else if (kDown & KEY_TOUCH)
		{
			if (pos.inRegion(0, 48, 8, 64*2+8)) disp = -1;
			else if (pos.inRegion(256-8, 48, 8, 64*2+8)) disp = +1;
		}

		if (!disp) break;

		int oldPage = page;

		page += disp;
		int nPages = bump.GetBumpCount();

		while (page < 0) page += nPages;
		while (page >= nPages) page -= nPages;

		if (page == oldPage)
			break;

		unloadPageIcons(oldPage);

		bump.SelectBump(page);
		updCursor();
		loadPageIcons();
	} while(0);

	// Selection code
	if (kDown & KEY_TOUCH) do
	{
		bool bFound = false;

		for (int i = 0; i < 6; i ++)
		{
			int xPos = 16 + (i % 3) * (64+16);
			int yPos = 48 + (i / 3) * (64+8);
			if (pos.inRegion(xPos, yPos, 64, 64))
			{
				int touchedApp = page*6 + i;
				if (touchedApp == selectedApp)
					executeApp();
				else if (g_appData[touchedApp].IsLoaded())
				{
					selectedApp = touchedApp;
					updCursor();
				}
				bFound = true;
				break;
			}
		}

		if (bFound) break;

		for (int i = 0; i < g_appList.GetCount(); i ++)
		{
			if (!pos.inRegion(64+(i*(16+8)), 16, 16, 16))
				continue;

			SwitchToApp(i);
			return true;
		}
	} while(0);

	if (kDown & KEY_A)
		executeApp();

#ifdef ALLOW_EXIT
	if (g_appList.GetCount() == 0 && (kDown & KEY_START))
		return false;
#endif

	return true;
}

static void RunAppVBlank()
{
	const AppInfo* info = g_curApp->GetInfo();
	word_t appFlags = info->Flags;

	word_t kDown = keysDown();

	if (!(appFlags & AppFlags::UsesSelect) && (kDown & KEY_SELECT))
	{
		g_curApp->OnDeactivate();
		g_curApp = nullptr;
		videoReset();
		videoInit();
		return;
	}

	g_curApp->OnVBlank();
}

static void RunBgProcess()
{
	if (g_appList.GetCount() == 0)
		return;

	g_appList.Iterate([&] (AppWrapper* app)
	{
		app->OnBgProcess();
		return true;
	});
}

#define ANSIESC_RED "\x1b[31;1m"
#define ANSIESC_GREEN "\x1b[32;1m"
#define ANSIESC_YELLOW "\x1b[33;1m"
#define ANSIESC_DEFAULT "\x1b[39;1m"

bool DoSplashScreen()
{
	CGrf* gfx = new CGrf();
	if (!gfx->Load(GUI_ASSET_DIR "/bootsplash.grf"))
	{
		delete gfx;
		return false;
	}

	FeOS_DirectMode();
	setBrightness(3, -16);

	videoSetMode(MODE_3_2D);
	int bg = bgInit(2, BgType_Text8bpp, BgSize_T_256x256, 0, 1);

	dmaCopy(gfx->gfxData, bgGetGfxPtr(bg), MemChunk_GetSize(gfx->gfxData));
	dmaCopy(gfx->mapData, bgGetMapPtr(bg), MemChunk_GetSize(gfx->mapData));
	dmaCopy(gfx->palData, BG_PALETTE,      MemChunk_GetSize(gfx->palData));
	delete gfx;
	return true;
}

int main()
{
	static bool bActive = false;

	if (bActive)
	{
		GetGuiManagerChecked()->SwitchTo(nullptr);
		return 0;
	}

	bActive = true;

	if (!DoSplashScreen())
	{
		printf(ANSIESC_RED "FAIL" ANSIESC_DEFAULT "\n");
		return 1;
	}

	LoadApps();
	if (!g_appCount)
	{
		FeOS_ConsoleMode();
		fprintf(stderr, "No applications!\n");
		return 1;
	}

	LoadFileTypes();

	for (int q = 0; q < 92; q ++)
	{
		if (q < 16)
			setBrightness(3, q-16);
		if (q >= (92-16-1))
			setBrightness(3, q-(92-16-1));
		if (keysDown()) break;
		swiWaitForVBlank();
	}

	setBrightness(3, 0);

	const modeshim_t* oldShim = FeOS_ModeShim(&modeShim);

	if (!(background.Load(GUI_ASSET_DIR "/background.grf") &&
		dummy.Load(GUI_ASSET_DIR "/dummy.grf") &&
		selection.Load(GUI_ASSET_DIR "/selection.grf") &&
		topscr.Load(GUI_ASSET_DIR "/topscr.grf") &&
		defappicon.Load(GUI_ASSET_DIR "/dummyapp.grf") &&
		fiGenericFile.Load(GUI_ASSET_DIR "/genericfile.grf") &&
		fiConflictFile.Load(GUI_ASSET_DIR "/conflictfile.grf") &&
		font.Load("tahoma", 10)))
	{
		FeOS_ModeShim(oldShim);
		FeOS_ConsoleMode();
		printf("FAIL\n");
		return 0;
	}

	swiWaitForVBlank();
	videoReset();
	videoInit();
	EnableGuiMon();

	for (;;)
	{
		swiWaitForVBlank();
		if (g_curApp)
			RunAppVBlank();
		else if (!MainVBlank())
			break;
		RunBgProcess();
	}

	DisableGuiMon();
	FeOS_ModeShim(oldShim);

	FeOS_ConsoleMode();
	return 0;
}
