#include "common.h"
#include <unistd.h>
#include <dirent.h>
#include <alloca.h>

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
		snprintf(buf, sizeof(buf), GUI_DIRECTORY "/%s", iconPath);
	buf[sizeof(buf)-1] = 0;

	cachedIcon = new CGrf();
	if (!cachedIcon->Load(buf))
	{
		FreeIcon();
		return nullptr;
	}

	return cachedIcon;
}

IFileIcon* CAppData::GetFileIcon()
{
	if (!this) return nullptr; // Yes, this is intentional.

	if (!cachedFileIcon.isLoaded())
	{
		const char* iconPath = GetFileIconPath();
		if (!iconPath || !*iconPath) return nullptr;

		char buf[256];
		if (*iconPath == '/')
			strncpy(buf, iconPath, sizeof(buf));
		else
			snprintf(buf, sizeof(buf), GUI_FTYPE_DIR "/%s", iconPath);
		buf[sizeof(buf)-1] = 0;

		CGrf* ic = new CGrf();
		if (!ic->Load(buf))
		{
			delete ic;
			return nullptr;
		}

		cachedFileIcon.data = ic;
	}

	return cachedFileIcon.getSelf();
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
		if (g_appData[g_appCount].Load(buf, pent->d_name))
			g_appCount ++;
	}

	closedir(d);
}

void resolveParam(char* buf, size_t bufSize, const char* cmd, const char* parm)
{
	*buf = 0;
	for (;;)
	{
		char* pos = strchr(cmd, '%');
		if (!pos)
		{
			strncat(buf, cmd, bufSize);
			break;
		}

		*pos = 0; // Shh...
		strncat(buf, cmd, bufSize);

		strncat(buf, "\"", bufSize);
		strncat(buf, parm, bufSize);
		strncat(buf, "\"", bufSize);

		*pos++ = '%';
		cmd = pos;
	}
}

void CAppData::Run(const char* parameter)
{
	bool isDirectMode = IsDirectMode();
	const char* cmd = parameter ? GetFileCommand() : GetCommand();
	if (!cmd) return;

	if (parameter)
	{
		char* buf = (char*)alloca(256);
		resolveParam(buf, 256, cmd, parameter);
		printf("%s\n", buf);
		cmd = buf;
	}

	if (isDirectMode)
	{
		DisableGuiMon();
		bRunningDMApp = true;
		system(cmd);
		bRunningDMApp = false;
		EnableGuiMon();
	}else
	{
		thread_t t = FeOS_RunAsync(cmd);
		if (!t) return;
		FeOS_DetachThread(t);
	}
}
