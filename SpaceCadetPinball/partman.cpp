#include "pch.h"
#include "partman.h"

short partman::_field_size[] = {
	2, -1, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0
};


datFileStruct* partman::load_records(LPCSTR lpFileName)
{
	_OFSTRUCT ReOpenBuff{};
	datFileHeader Buffer{};
	dat8BitBmpHeader bmpHeader{};
	datFileStruct* datFile;
	int groupIndex, groupDataSize;
	datGroupData** groupDataBuf;

	const HFILE fileHandle = OpenFile(lpFileName, &ReOpenBuff, 0);
	if (fileHandle == -1)
		return nullptr;
	_lread(fileHandle, &Buffer, 183u);
	if (lstrcmpA("PARTOUT(4.0)RESOURCE", Buffer.FileSignature))
	{
		_lclose(fileHandle);
		return nullptr;
	}
	datFile = (datFileStruct*)memoryallocate(sizeof(datFileStruct));
	if (!datFile)
	{
		_lclose(fileHandle);
		return nullptr;
	}
	if (lstrlenA(Buffer.Description) <= 0)
	{
		datFile->Description = nullptr;
	}
	else
	{
		int lenOfStr = lstrlenA(Buffer.Description);
		auto descriptionBuf = (char*)memoryallocate(lenOfStr + 1);
		datFile->Description = descriptionBuf;
		if (!descriptionBuf)
		{
			_lclose(fileHandle);
			memoryfree(datFile);
			return nullptr;
		}
		lstrcpyA(descriptionBuf, Buffer.Description);
	}

	if (Buffer.Unknown)
	{
		auto unknownBuf = (char*)memoryallocate(Buffer.Unknown);
		if (!unknownBuf)
		{
			_lclose(fileHandle);
			if (datFile->Description)
				memoryfree(datFile->Description);
			memoryfree(datFile);
			return nullptr;
		}
		_lread(fileHandle, static_cast<void*>(unknownBuf), Buffer.Unknown);
		memoryfree(unknownBuf);
	}

	groupDataBuf = (datGroupData**)memoryallocate(sizeof(void*) * Buffer.NumberOfGroups);
	datFile->GroupData = groupDataBuf;
	if (!groupDataBuf)
	{
		if (datFile->Description)
			memoryfree(datFile->Description);
		memoryfree(datFile);
		return nullptr;
	}

	groupIndex = 0;
	if (Buffer.NumberOfGroups)
	{
		do
		{
			char entryCount = _lread_char(fileHandle);
			if (entryCount <= 0)
				groupDataSize = 0;
			else
				groupDataSize = entryCount - 1;
			datFile->GroupData[groupIndex] = (datGroupData*)memoryallocate(
				sizeof(datEntryData) * groupDataSize + sizeof(datGroupData));
			datGroupData* groupData = datFile->GroupData[groupIndex];
			if (!groupData)
				break;
			int entryIndex = 0;
			groupData->EntryCount = entryCount;
			if (entryCount > 0)
			{
				datEntryData* entryData = groupData->Entries;
				do
				{
					auto entryType = static_cast<datFieldTypes>(_lread_char(fileHandle));
					entryData->EntryType = entryType;
					int fieldSize = _field_size[entryType];
					if (fieldSize < 0)
					{
						fieldSize = _lread_long(fileHandle);
					}
					if (entryType == Bitmap8bit)
					{
						_hread(fileHandle, &bmpHeader, 14);
						char* bmpBuffer = (char*)memoryallocate(0x25u);
						entryData->Buffer = bmpBuffer;
						if (!bmpBuffer)
							goto LABEL_41;
						/*if (bmpHeader.Unknown2 & 2 ? gdrv_create_bitmap((int)bmpBuffer, bmpHeader.Width, bmpHeader.Height) : gdrv_create_raw_bitmap((int)bmpBuffer, bmpHeader.Width, bmpHeader.Height, bmpHeader.Unknown2 & 1))
							goto LABEL_41;*/
						//_hread(fileHandle, *(LPVOID*)(entryData->Buffer + 8), bmpHeader.Size);
						char* tempBuff = (char*)memoryallocate(bmpHeader.Size);
						_hread(fileHandle, tempBuff, bmpHeader.Size);
						memoryfree(tempBuff);
						//*((int*)entryData->Buffer + 29) = bmpHeader.XPosition;
						//*((int*)entryData->Buffer + 33) = bmpHeader.YPosition;
					}
					else
					{						
						char* entryBuffer = (char*)memoryallocate(fieldSize);
						entryData->Buffer = entryBuffer;
						if (!entryBuffer)
							goto LABEL_41;
						_hread(fileHandle, entryBuffer, fieldSize);
					}

					++entryIndex;
					entryData->FieldSize = fieldSize;
					datFile->NumberOfGroups = groupIndex + 1;
					++entryData;
				}
				while (entryIndex < entryCount);
			}
			++groupIndex;
		}
		while (groupIndex < Buffer.NumberOfGroups);
	}

LABEL_41:
	_lclose(fileHandle);
	if (datFile->NumberOfGroups == Buffer.NumberOfGroups)
		return datFile;
	unload_records(datFile);
	return nullptr;
}


void partman::unload_records(datFileStruct* datFile)
{
	for (int groupIndex = 0; groupIndex < datFile->NumberOfGroups; ++groupIndex)
	{
		datGroupData* group = datFile->GroupData[groupIndex];
		if (group)
		{
			int entryIndex = 0;
			if (group->EntryCount > 0)
			{
				datEntryData* entry = group->Entries;
				do
				{
					if (entry->Buffer)
					{
						//if (HIWORD(entry->EntryType) == 1)
						//gdrv_destroy_bitmap(entry->Buffer);
						memoryfree(entry->Buffer);
					}
					++entryIndex;
					++entry;
				}
				while (entryIndex < group->EntryCount);
			}
			memoryfree(group);
		}
	}
	if (datFile->Description)
		memoryfree(datFile->Description);
	memoryfree(datFile->GroupData);
	memoryfree(datFile);
}

char* partman::field(datFileStruct* datFile, int groupIndex, datFieldTypes targetEntryType)
{
	datGroupData* groupData = datFile->GroupData[groupIndex];
	int entryCount = groupData->EntryCount;
	int entryIndex = 0;
	if (entryCount <= 0)
		return nullptr;
	datEntryData* entry = groupData->Entries;
	while (true)
	{
		int entryType = entry->EntryType;
		if (entryType == targetEntryType)
			break;
		if (entryType > targetEntryType)
			return nullptr;
		++entryIndex;
		++entry;
		if (entryIndex < entryCount)
			continue;
		return nullptr;
	}
	return entry->Buffer;
}


char* partman::field_nth(datFileStruct* datFile, int groupIndex, datFieldTypes targetEntryType, int skipFirstN)
{
	datGroupData* groupData = datFile->GroupData[groupIndex];
	int entryCount = groupData->EntryCount, skipCount = 0, entryIndex = 0;
	if (0 < entryCount)
	{
		datEntryData* entry = groupData->Entries;
		do
		{
			int entryType = entry->EntryType;
			if (entryType == targetEntryType)
			{
				if (skipCount == skipFirstN)
				{
					return entry->Buffer;
				}
				skipCount++;
			}
			else
			{
				if (targetEntryType < entryType)
				{
					return nullptr;
				}
			}
			entryIndex++;
			entry++;
		}
		while (entryIndex < entryCount);
	}
	return nullptr;
}

int partman::field_size_nth(datFileStruct* datFile, int groupIndex, datFieldTypes targetEntryType, int skipFirstN)
{
	datGroupData* groupData = datFile->GroupData[groupIndex];
	int entryCount = groupData->EntryCount, skipCount = 0, entryIndex = 0;
	if (0 < entryCount)
	{
		datEntryData* entry = groupData->Entries;
		do
		{
			int entryType = entry->EntryType;
			if (entryType == targetEntryType)
			{
				if (skipCount == skipFirstN)
				{
					return entry->FieldSize;
				}
				skipCount++;
			}
			else
			{
				if (targetEntryType < entryType)
				{
					return 0;
				}
			}
			entryIndex++;
			entry++;
		}
		while (entryIndex < entryCount);
	}
	return 0;
}

int partman::field_size(datFileStruct* datFile, int groupIndex, datFieldTypes targetEntryType)
{
	return field_size_nth(datFile, groupIndex, targetEntryType, 0);
}

int partman::record_labeled(datFileStruct* datFile, LPCSTR targetGroupName)
{
	int trgGroupNameLen = lstrlenA(targetGroupName);
	int groupIndex = datFile->NumberOfGroups;
	while (true)
	{
		if (--groupIndex < 0)
			return -1;
		char* groupName = field(datFile, groupIndex, GroupName);
		if (groupName)
		{
			int index = 0;
			bool found = trgGroupNameLen == 0;
			if (trgGroupNameLen > 0)
			{
				LPCSTR targetNamePtr = targetGroupName;
				do
				{
					if (*targetNamePtr != targetNamePtr[groupName - targetGroupName])
						break;
					++index;
					++targetNamePtr;
				}
				while (index < trgGroupNameLen);
				found = index == trgGroupNameLen;
			}
			if (found && !targetGroupName[index] && !groupName[index])
				break;
		}
	}
	return groupIndex;
}

char* partman::field_labeled(datFileStruct* datFile, LPCSTR lpString, datFieldTypes fieldType)
{
	char* result;
	int groupIndex = record_labeled(datFile, lpString);
	if (groupIndex < 0)
		result = nullptr;
	else
		result = field(datFile, groupIndex, fieldType);
	return result;
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

char partman::_lread_char(HFILE hFile)
{
	char Buffer = 0;
	_lread(hFile, &Buffer, 1u);
	return Buffer;
}

int partman::_lread_long(HFILE hFile)
{
	int Buffer = 0;
	_lread(hFile, &Buffer, 4u);
	return Buffer;
}
