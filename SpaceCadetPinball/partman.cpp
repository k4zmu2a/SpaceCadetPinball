#include "pch.h"
#include "partman.h"

short partman::_field_size[] = { 2, 0x0FFFF, 2, 0x0FFFF, 0x0FFFF, 0x0FFFF, 0x0FFFF, 0x0FFFF,0x0FFFF, 0x0FFFF, 0x0FFFF, 0x0FFFF, 0x0FFFF, 0 };


datFileStruct* partman::load_records(LPCSTR lpFileName)
{
	_OFSTRUCT ReOpenBuff{};
	datFileHeader Buffer;
	datFileStruct* datFile;
	HFILE fileHandle, hFile;
	int lenOfStr, groupIndex;
	unsigned short unknown;
	char* descriptionBuf, * unknownBuf, * unknownBuf2;
	char** groupDataBuf;

	fileHandle = OpenFile(lpFileName, &ReOpenBuff, 0);
	hFile = fileHandle;
	if (fileHandle == -1)
		return 0;
	_lread(fileHandle, &Buffer, 183u);
	if (lstrcmpA("PARTOUT(4.0)RESOURCE", Buffer.FileSignature))
	{
		_lclose(fileHandle);
		return 0;
	}
	datFile = (datFileStruct*)memoryallocate(10);
	if (!datFile)
	{
		_lclose(fileHandle);
		return 0;
	}
	if (lstrlenA(Buffer.Description) <= 0)
	{
		datFile->Description = 0;
	}
	else
	{
		lenOfStr = lstrlenA(Buffer.Description);
		descriptionBuf = (char*)memoryallocate(lenOfStr + 1);
		datFile->Description = descriptionBuf;
		if (!descriptionBuf)
		{
			_lclose(fileHandle);
		LABEL_10:
			memoryfree(datFile);
			return 0;
		}
		lstrcpyA(descriptionBuf, Buffer.Description);
	}
	unknown = Buffer.Unknown;
	if (Buffer.Unknown)
	{
		unknownBuf = (char*)memoryallocate(Buffer.Unknown);
		unknownBuf2 = unknownBuf;
		if (!unknownBuf)
		{
			_lclose(hFile);
			goto LABEL_19;
		}
		_lread(hFile, (void*)unknownBuf, unknown);
		memoryfree(unknownBuf2);
	}
	groupDataBuf = (char**)memoryallocate(4 * Buffer.NumberOfGroups);
	datFile->GroupData = groupDataBuf;
	if (!groupDataBuf)
	{
	LABEL_19:
		if (datFile->Description)
			memoryfree(datFile->Description);
		goto LABEL_10;
	}
	groupIndex = 0;


	return datFile;
}


int partman::make_path_name(LPSTR lpFilename, LPCSTR lpString2, int nSize)
{
	int nameSize = GetModuleFileNameA(nullptr, lpFilename, nSize);
	if (!nameSize || nameSize == nSize)
		return 1;
	for (CHAR* i = &lpFilename[nameSize]; i > lpFilename; --i)
	{
		if (*i == '\\' || *i == ':')
		{
			i[1] = 0;
			break;
		}
		--nameSize;
	}
	if (nameSize + 13 < nSize)
	{
		lstrcatA(lpFilename, lpString2);
		return 0;
	}
	lstrcatA(lpFilename, "?");
	return 1;
}