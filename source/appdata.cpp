#include "common.h"
#include <unistd.h>
#include <dirent.h>

CGrf* CAppData::GetIcon()
{
	if (!bLoaded) return nullptr;
	if (cachedIcon) return cachedIcon;

	const char* iconPath = GetIconPath();
	if (!iconPath || !*iconPath) return nullptr;

	char buf[256];
	if (*iconPath == '/')
		strncpy(buf, iconPath, sizeof(buf));
	else
		snprintf(buf, sizeof(buf), "/data/FeOS/gui/%s", iconPath);
	buf[sizeof(buf)-1] = 0;

	cachedIcon = new CGrf();
	if (!cachedIcon->Load(buf))
	{
		FreeIcon();
		return nullptr;
	}

	return cachedIcon;
}

void LoadApps()
{
	DIR* d;
	struct dirent* pent;

	g_appCount = 0;

	if (!(d = opendir(GUI_DIRECTORY)))
		return;

	while ((pent = readdir(d)) != NULL)
	{
		if (stricmp(pent->d_name + strlen(pent->d_name) -4, ".app") != 0)
			continue;
		char buf[256];
		snprintf(buf, sizeof(buf), GUI_DIRECTORY "/%s", pent->d_name);
		buf[sizeof(buf)-1] = 0;
		if (g_appData[g_appCount].Load(buf))
			g_appCount ++;
	}

	closedir(d);
}
