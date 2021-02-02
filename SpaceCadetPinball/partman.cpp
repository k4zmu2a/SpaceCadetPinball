#include "pch.h"
#include "partman.h"
#include "gdrv.h"
#include "memory.h"
#include "zdrv.h"

short partman::_field_size[] =
{
	2, -1, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0
};

datFileStruct* partman::load_records(LPCSTR lpFileName, int resolution, bool fullTiltMode)
{
	_OFSTRUCT ReOpenBuff{};
	datFileHeader header{};
	dat8BitBmpHeader bmpHeader{};
	dat16BitBmpHeader zMapHeader{};

	const HFILE fileHandle = OpenFile(lpFileName, &ReOpenBuff, 0);
	if (fileHandle == -1)
		return nullptr;
	_lread(fileHandle, &header, 183u);
	if (lstrcmpA("PARTOUT(4.0)RESOURCE", header.FileSignature))
	{
		_lclose(fileHandle);
		return nullptr;
	}
	auto datFile = memory::allocate<datFileStruct>();
	if (!datFile)
	{
		_lclose(fileHandle);
		return nullptr;
	}
	if (lstrlenA(header.Description) <= 0)
	{
		datFile->Description = nullptr;
	}
	else
	{
		int lenOfStr = lstrlenA(header.Description);
		auto descriptionBuf = memory::allocate(lenOfStr + 1);
		datFile->Description = descriptionBuf;
		if (!descriptionBuf)
		{
			_lclose(fileHandle);
			memory::free(datFile);
			return nullptr;
		}
		lstrcpyA(descriptionBuf, header.Description);
	}

	if (header.Unknown)
	{
		auto unknownBuf = memory::allocate(header.Unknown);
		if (!unknownBuf)
		{
			_lclose(fileHandle);
			if (datFile->Description)
				memory::free(datFile->Description);
			memory::free(datFile);
			return nullptr;
		}
		_lread(fileHandle, static_cast<void*>(unknownBuf), header.Unknown);
		memory::free(unknownBuf);
	}

	auto groupDataBuf = memory::allocate<datGroupData*>(header.NumberOfGroups);
	datFile->GroupData = groupDataBuf;
	if (!groupDataBuf)
	{
		if (datFile->Description)
			memory::free(datFile->Description);
		memory::free(datFile);
		return nullptr;
	}

	bool abort = false;
	for (auto groupIndex = 0; !abort && groupIndex < header.NumberOfGroups; ++groupIndex)
	{
		auto entryCount = _lread_char(fileHandle);
		auto groupDataSize = entryCount <= 0 ? 0 : entryCount - 1;
		auto groupData = memory::allocate<datGroupData>(1, sizeof(datEntryData) * groupDataSize);
		datFile->GroupData[groupIndex] = groupData;
		if (!groupData)
			break;

		groupData->EntryCount = 0;
		for (auto entryIndex = 0; entryIndex < entryCount; ++entryIndex)
		{
			auto entryData = &groupData->Entries[groupData->EntryCount];
			auto entryType = static_cast<datFieldTypes>(_lread_char(fileHandle));
			entryData->EntryType = entryType;

			int fieldSize = _field_size[static_cast<int>(entryType)];
			if (fieldSize < 0)
				fieldSize = _lread_long(fileHandle);

			if (entryType == datFieldTypes::Bitmap8bit)
			{
				_hread(fileHandle, &bmpHeader, sizeof(dat8BitBmpHeader));
				if (bmpHeader.Resolution != resolution && bmpHeader.Resolution != -1)
				{
					_llseek(fileHandle, bmpHeader.Size, 1);
					continue;
				}

				auto bmp = memory::allocate<gdrv_bitmap8>();
				entryData->Buffer = reinterpret_cast<char*>(bmp);
				if (!bmp)
				{
					abort = true;
					break;
				}
				int bmpRez;
				if (bmpHeader.IsFlagSet(bmp8Flags::Spliced))
					bmpRez = gdrv::create_spliced_bitmap(bmp, bmpHeader.Width, bmpHeader.Height, bmpHeader.Size);
				else if (bmpHeader.IsFlagSet(bmp8Flags::DibBitmap))
					bmpRez = gdrv::create_bitmap(bmp, bmpHeader.Width, bmpHeader.Height);
				else
					bmpRez = gdrv::create_raw_bitmap(bmp, bmpHeader.Width, bmpHeader.Height,
					                                 bmpHeader.IsFlagSet(bmp8Flags::RawBmpUnaligned));
				if (bmpRez)
				{
					abort = true;
					break;
				}
				_hread(fileHandle, bmp->BmpBufPtr1, bmpHeader.Size);
				bmp->XPosition = bmpHeader.XPosition;
				bmp->YPosition = bmpHeader.YPosition;
			}
			else if (entryType == datFieldTypes::Bitmap16bit)
			{
				/*Full tilt has extra byte(@0:resolution) in zMap*/
				if (fullTiltMode)
				{
					char zMapResolution = _lread_char(fileHandle);
					fieldSize--;
					if (zMapResolution != resolution && zMapResolution != -1)
					{
						_llseek(fileHandle, fieldSize, 1);
						continue;
					}
				}

				_hread(fileHandle, &zMapHeader, sizeof(dat16BitBmpHeader));
				int length = fieldSize - sizeof(dat16BitBmpHeader);

				auto zmap = memory::allocate<zmap_header_type>(1, length);
				zmap->Width = zMapHeader.Width;
				zmap->Height = zMapHeader.Height;
				zmap->Stride = zMapHeader.Stride;
				_hread(fileHandle, zmap->ZBuffer, length);
				entryData->Buffer = reinterpret_cast<char*>(zmap);
			}
			else
			{
				char* entryBuffer = memory::allocate(fieldSize);
				entryData->Buffer = entryBuffer;
				if (!entryBuffer)
				{
					abort = true;
					break;
				}
				_hread(fileHandle, entryBuffer, fieldSize);
			}

			entryData->FieldSize = fieldSize;
			groupData->EntryCount++;
		}
		datFile->NumberOfGroups = groupIndex + 1;
	}

	_lclose(fileHandle);
	if (datFile->NumberOfGroups == header.NumberOfGroups)
		return datFile;
	unload_records(datFile);
	return nullptr;
}


void partman::unload_records(datFileStruct* datFile)
{
	for (auto groupIndex = 0; groupIndex < datFile->NumberOfGroups; ++groupIndex)
	{
		auto group = datFile->GroupData[groupIndex];
		if (!group)
			continue;

		for (auto entryIndex = 0; entryIndex < group->EntryCount; ++entryIndex)
		{
			auto entry = &group->Entries[entryIndex];
			if (entry->Buffer)
			{
				if (entry->EntryType == datFieldTypes::Bitmap8bit)
					gdrv::destroy_bitmap(reinterpret_cast<gdrv_bitmap8*>(entry->Buffer));
				memory::free(entry->Buffer);
			}
		}
		memory::free(group);
	}
	if (datFile->Description)
		memory::free(datFile->Description);
	memory::free(datFile->GroupData);
	memory::free(datFile);
}

char* partman::field(datFileStruct* datFile, int groupIndex, datFieldTypes targetEntryType)
{
	auto group = datFile->GroupData[groupIndex];
	for (auto entryIndex = 0; entryIndex < group->EntryCount; ++entryIndex)
	{
		auto entry = &group->Entries[entryIndex];
		if (entry->EntryType == targetEntryType)
			return entry->Buffer;
		if (entry->EntryType > targetEntryType)
			break;
	}
	return nullptr;
}


char* partman::field_nth(datFileStruct* datFile, int groupIndex, datFieldTypes targetEntryType, int skipFirstN)
{
	auto group = datFile->GroupData[groupIndex];
	for (auto skipCount = 0, entryIndex = 0; entryIndex < group->EntryCount; ++entryIndex)
	{
		auto entry = &group->Entries[entryIndex];
		if (entry->EntryType > targetEntryType)
			break;
		if (entry->EntryType == targetEntryType)
			if (skipCount++ == skipFirstN)
				return entry->Buffer;
	}
	return nullptr;
}

int partman::field_size_nth(datFileStruct* datFile, int groupIndex, datFieldTypes targetEntryType, int skipFirstN)
{
	auto group = datFile->GroupData[groupIndex];
	for (auto skipCount = 0, entryIndex = 0; entryIndex < group->EntryCount; ++entryIndex)
	{
		auto entry = &group->Entries[entryIndex];
		if (entry->EntryType > targetEntryType)
			return 0;
		if (entry->EntryType == targetEntryType)
			if (skipCount++ == skipFirstN)
				return entry->FieldSize;
	}
	return 0;
}

int partman::field_size(datFileStruct* datFile, int groupIndex, datFieldTypes targetEntryType)
{
	return field_size_nth(datFile, groupIndex, targetEntryType, 0);
}

int partman::record_labeled(datFileStruct* datFile, LPCSTR targetGroupName)
{
	auto targetLength = lstrlenA(targetGroupName);
	for (int groupIndex = datFile->NumberOfGroups - 1; groupIndex >= 0; --groupIndex)
	{
		auto groupName = field(datFile, groupIndex, datFieldTypes::GroupName);
		if (!groupName)
			continue;

		int index;
		for (index = 0; index < targetLength; index++)
			if (targetGroupName[index] != groupName[index])
				break;
		if (index == targetLength && !targetGroupName[index] && !groupName[index])
			return groupIndex;
	}

	return -1;
}

char* partman::field_labeled(datFileStruct* datFile, LPCSTR lpString, datFieldTypes fieldType)
{
	auto groupIndex = record_labeled(datFile, lpString);
	return groupIndex < 0 ? nullptr : field(datFile, groupIndex, fieldType);
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
