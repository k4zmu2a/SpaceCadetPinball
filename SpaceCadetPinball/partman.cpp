#include "pch.h"
#include "partman.h"

#include "fullscrn.h"
#include "gdrv.h"
#include "GroupData.h"
#include "memory.h"
#include "zdrv.h"

short partman::_field_size[] =
{
	2, -1, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0
};

DatFile* partman::load_records(LPCSTR lpFileName, bool fullTiltMode)
{
	datFileHeader header{};
	dat8BitBmpHeader bmpHeader{};
	dat16BitBmpHeader zMapHeader{};

	auto fileHandle = fopen(lpFileName, "rb");
	if (fileHandle == nullptr)
		return nullptr;

	fread(&header, 1, sizeof header, fileHandle);
	if (strcmp("PARTOUT(4.0)RESOURCE", header.FileSignature) != 0)
	{
		fclose(fileHandle);
		return nullptr;
	}

	auto datFile = new DatFile();
	if (!datFile)
	{
		fclose(fileHandle);
		return nullptr;
	}

	datFile->AppName = header.AppName;
	datFile->Description = header.Description;

	if (header.Unknown)
	{
		auto unknownBuf = memory::allocate(header.Unknown);
		if (!unknownBuf)
		{
			fclose(fileHandle);
			delete datFile;
			return nullptr;
		}
		fread(unknownBuf, 1, header.Unknown, fileHandle);
		memory::free(unknownBuf);
	}

	datFile->Groups.reserve(header.NumberOfGroups);
	bool abort = false;
	for (auto groupIndex = 0; !abort && groupIndex < header.NumberOfGroups; ++groupIndex)
	{
		auto entryCount = _lread_char(fileHandle);
		auto groupData = new GroupData(groupIndex);
		groupData->ReserveEntries(entryCount);

		for (auto entryIndex = 0; entryIndex < entryCount; ++entryIndex)
		{
			auto entryData = new EntryData();
			auto entryType = static_cast<FieldTypes>(_lread_char(fileHandle));
			entryData->EntryType = entryType;

			int fieldSize = _field_size[static_cast<int>(entryType)];
			if (fieldSize < 0)
				fieldSize = _lread_long(fileHandle);
			entryData->FieldSize = fieldSize;

			if (entryType == FieldTypes::Bitmap8bit)
			{
				fread(&bmpHeader, 1, sizeof(dat8BitBmpHeader), fileHandle);
				assertm(bmpHeader.Size + sizeof(dat8BitBmpHeader) == fieldSize, "partman: Wrong bitmap field size");
				assertm(bmpHeader.Resolution >= 0 && bmpHeader.Resolution <= 2,
				        "partman: bitmap resolution out of bounds");

				auto bmp = memory::allocate<gdrv_bitmap8>();
				entryData->Buffer = reinterpret_cast<char*>(bmp);
				if (!bmp || gdrv::create_bitmap(*bmp, bmpHeader))
				{
					abort = true;
					break;
				}
				fread(bmp->IndexedBmpPtr, 1, bmpHeader.Size, fileHandle);
			}
			else if (entryType == FieldTypes::Bitmap16bit)
			{
				/*Full tilt has extra byte(@0:resolution) in zMap*/
				char zMapResolution = 0;
				if (fullTiltMode)
				{
					zMapResolution = _lread_char(fileHandle);
					fieldSize--;
					assertm(zMapResolution >= 0 && zMapResolution <= 2, "partman: zMap resolution out of bounds");
				}

				fread(&zMapHeader, 1, sizeof(dat16BitBmpHeader), fileHandle);
				int length = fieldSize - sizeof(dat16BitBmpHeader);

				auto zMap = memory::allocate<zmap_header_type>();
				zdrv::create_zmap(zMap, zMapHeader.Width, zMapHeader.Height, zMapHeader.Stride);
				zMap->Resolution = zMapResolution;
				if (zMapHeader.Stride * zMapHeader.Height * 2 == length)
				{
					fread(zMap->ZPtr1, 1, length, fileHandle);
				}
				else
				{
					// 3DPB .dat has zeroed zMap headers, in groups 497 and 498, skip them.
					fseek(fileHandle, length, SEEK_CUR);
				}
				entryData->Buffer = reinterpret_cast<char*>(zMap);
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
				fread(entryBuffer, 1, fieldSize, fileHandle);
			}

			groupData->AddEntry(entryData);
		}

		groupData->FinalizeGroup();
		datFile->Groups.push_back(groupData);
	}

	fclose(fileHandle);
	if (datFile->Groups.size() == header.NumberOfGroups)
		return datFile;
	delete datFile;
	return nullptr;
}

char partman::_lread_char(FILE* file)
{
	char Buffer = 0;
	fread(&Buffer, 1, 1, file);
	return Buffer;
}

int partman::_lread_long(FILE* file)
{
	int Buffer = 0;
	fread(&Buffer, 1, 4, file);
	return Buffer;
}
