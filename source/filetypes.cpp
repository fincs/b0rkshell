#include "common.h"
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>

static void addFileType(const char* ext, const char* app, const char* descr)
{
	// TODO: better app searching
	CAppData* pApp = nullptr;
	if (app)
		for (int i = 0; i < g_appCount; i ++)
			if (stricmp(g_appData[i].GetName(), app) == 0)
			{
				pApp = g_appData + i;
				break;
			}

	//if (!pApp)
	//	return;

	CFileType* ft = new CFileType();
	if (!ft) return;

	strncpy(ft->ext, ext, sizeof(ft->ext));
	ft->opensWith = pApp;
	ft->description = strdup(descr);

	if (g_firstFileType)
		g_firstFileType->Chain(ft);
	g_firstFileType = ft;
	g_filetypeCount ++;
}

static char* trim_whitespace(char* buf)
{
	// Remove trailing whitespace
	int pos;
	for(pos = strlen(buf)-1; pos >= 0 && isspace(buf[pos]); pos --) buf[pos] = '\0';

	// Remove leading whitespace
	char* newbuf = buf;
	for(; isspace(*newbuf); newbuf ++);

	return newbuf;
}

static void processCfgFile(const char* fname)
{
	FILE* f = fopen(fname, "r");
	char buf[256];

	while (!feof(f) && fgets(buf, sizeof(buf), f))
	{
		char* line = trim_whitespace(buf);
		if (!*line)
			continue;

		char* app = strchr(line, '=');
		if (!app)
			continue;

		*app = 0;

		char* ext = trim_whitespace(line);

		if (!*ext)
			continue;

		char* descr = strchr(++app, ',');
		if (!descr)
			continue;

		*descr = 0;
		app = trim_whitespace(app);
		descr = trim_whitespace(descr + 1);

		if (/*!*app ||*/ !*descr)
			continue;

		addFileType(ext, *app ? app : nullptr, descr);
	}

	fclose(f);
}

static void readFileTypes()
{
	DIR* d;
	struct dirent* pent;

	g_filetypeCount = 0;

	if (!(d = opendir(GUI_FTYPE_DIR)))
		return;

	while ((pent = readdir(d)) != NULL)
	{
		if (stricmp(pent->d_name + strlen(pent->d_name) -4, ".cfg") != 0)
			continue;
		char buf[256];
		snprintf(buf, sizeof(buf), GUI_FTYPE_DIR "/%s", pent->d_name);
		buf[sizeof(buf)-1] = 0;
		processCfgFile(buf);
	}

	closedir(d);
}

static int compareFT(const void* pa, const void* pb)
{
	CFileType *a = *(CFileType**)pa, *b = *(CFileType**)pb;
	u64 va = a->extId, vb = b->extId;
	if (va == vb) return 0;
	if (va > vb) return 1;
	return -1;
}

void LoadFileTypes()
{
	readFileTypes();

	// Prepare array of filetypes
	if (!g_filetypeCount)
		return;

	g_fileTypes = (CFileType**) malloc(sizeof(CFileType*) * g_filetypeCount);
	CFileType* q = g_firstFileType;
	for (int i = 0; i < g_filetypeCount; i ++)
	{
		g_fileTypes[i] = q;
		q = q->next;
	}

	qsort(g_fileTypes, g_filetypeCount, sizeof(CFileType*), compareFT);
	for (int i = 0; i < g_filetypeCount; i ++)
		g_fileTypes[i]->metaPtr = g_fileTypes + i;
}

static CFileType* rawFileTypeFind(u64 ftId)
{
	int min = 0, max = g_filetypeCount-1;

	while (max >= min)
	{
		int mid = (min + max) / 2;
		CFileType* t = g_fileTypes[mid];

		if (t->extId < ftId)
			min = mid + 1;
		else if (t->extId > ftId)
			max = mid - 1;
		else
			return t;
	}
	return nullptr;
}

CFileType* FindFileType(const char* ext, bool& multi)
{
	union
	{
		char tmpBuf[8];
		u64 ftId;
	};

	multi = false;

	if (!g_filetypeCount)
		return nullptr;

	ftId = 0;
	char *cBuf = tmpBuf, *cEnd = tmpBuf + sizeof(tmpBuf);
	while (*ext && cBuf < cEnd)
		*cBuf++ = tolower(*ext++);

	CFileType* f = rawFileTypeFind(ftId);
	if (!f) return nullptr;

	CFileType** p = f->metaPtr;
	CFileType **pt = g_fileTypes;

	if ((p > pt && p[-1]->extId == ftId) || (p < (pt + g_filetypeCount - 1) && p[1]->extId == ftId))
		multi = true;

	return f;
}

FEOSFINI void freeFileTypes()
{
	for (int i = 0; i < g_filetypeCount; i ++)
		delete g_fileTypes[i];
	if (g_fileTypes) free(g_fileTypes);
}
