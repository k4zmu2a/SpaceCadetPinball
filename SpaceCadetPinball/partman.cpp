#include "pch.h"
#include "partman.h"

#include "gdrv.h"
#include "GroupData.h"
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

	auto fileHandle = fopenu(lpFileName, "rb");
	if (fileHandle == nullptr)
		return nullptr;

	fread(&header, 1, sizeof header, fileHandle);

	header.FileSize = SDL_SwapLE32(header.FileSize);
	header.NumberOfGroups = SDL_SwapLE16(header.NumberOfGroups);
	header.SizeOfBody = SDL_SwapLE32(header.SizeOfBody);
	header.Unknown = SDL_SwapLE16(header.Unknown);

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
		auto unknownBuf = new char[header.Unknown];
		if (!unknownBuf)
		{
			fclose(fileHandle);
			delete datFile;
			return nullptr;
		}
		fread(unknownBuf, 1, header.Unknown, fileHandle);
		delete[] unknownBuf;
	}

	datFile->Groups.reserve(header.NumberOfGroups);
	bool abort = false;
	for (auto groupIndex = 0; !abort && groupIndex < header.NumberOfGroups; ++groupIndex)
	{
		auto entryCount = LRead<uint8_t>(fileHandle);
		auto groupData = new GroupData(groupIndex);
		groupData->ReserveEntries(entryCount);

		for (auto entryIndex = 0; entryIndex < entryCount; ++entryIndex)
		{
			auto entryData = new EntryData();
			auto entryType = static_cast<FieldTypes>(LRead<uint8_t>(fileHandle));
			entryData->EntryType = entryType;

			int fixedSize = _field_size[static_cast<int>(entryType)];
			size_t fieldSize = fixedSize >= 0 ? fixedSize : LRead<uint32_t>(fileHandle);
			entryData->FieldSize = static_cast<int>(fieldSize);

			if (entryType == FieldTypes::Bitmap8bit)
			{
				fread(&bmpHeader, 1, sizeof(dat8BitBmpHeader), fileHandle);
				bmpHeader.Width = SDL_SwapLE16(bmpHeader.Width);
				bmpHeader.Height = SDL_SwapLE16(bmpHeader.Height);
				bmpHeader.XPosition = SDL_SwapLE16(bmpHeader.XPosition);
				bmpHeader.YPosition = SDL_SwapLE16(bmpHeader.YPosition);
				bmpHeader.Size = SDL_SwapLE32(bmpHeader.Size);
				assertm(bmpHeader.Size + sizeof(dat8BitBmpHeader) == fieldSize, "partman: Wrong bitmap field size");
				assertm(bmpHeader.Resolution <= 2, "partman: bitmap resolution out of bounds");

				auto bmp = new gdrv_bitmap8(bmpHeader);
				entryData->Buffer = reinterpret_cast<char*>(bmp);
				fread(bmp->IndexedBmpPtr, 1, bmpHeader.Size, fileHandle);
			}
			else if (entryType == FieldTypes::Bitmap16bit)
			{
				/*Full tilt has extra byte(@0:resolution) in zMap*/
				auto zMapResolution = 0u;
				if (fullTiltMode)
				{
					zMapResolution = LRead<uint8_t>(fileHandle);
					fieldSize--;
					assertm(zMapResolution <= 2, "partman: zMap resolution out of bounds");
				}

				fread(&zMapHeader, 1, sizeof(dat16BitBmpHeader), fileHandle);
				zMapHeader.Width = SDL_SwapLE16(zMapHeader.Width);
				zMapHeader.Height = SDL_SwapLE16(zMapHeader.Height);
				zMapHeader.Stride = SDL_SwapLE16(zMapHeader.Stride);
				zMapHeader.Unknown0 = SDL_SwapLE32(zMapHeader.Unknown0);
				zMapHeader.Unknown1_0 = SDL_SwapLE16(zMapHeader.Unknown1_0);
				zMapHeader.Unknown1_1 = SDL_SwapLE16(zMapHeader.Unknown1_1);
				auto length = fieldSize - sizeof(dat16BitBmpHeader);

				auto zMap = new zmap_header_type(zMapHeader.Width, zMapHeader.Height, zMapHeader.Stride);
				zMap->Resolution = zMapResolution;
				if (zMapHeader.Stride * zMapHeader.Height * 2u == length)
				{
					fread(zMap->ZPtr1, 1, length, fileHandle);
					for (int i = 0; i < zMapHeader.Stride * zMapHeader.Height; i++) {
						zMap->ZPtr1[i] = SDL_SwapLE16(zMap->ZPtr1[i]);
					}
				}
				else
				{
					// 3DPB .dat has zeroed zMap headers, in groups 497 and 498, skip them.
					fseek(fileHandle, static_cast<int>(length), SEEK_CUR);
				}
				entryData->Buffer = reinterpret_cast<char*>(zMap);
			}
			else
			{
				auto entryBuffer = new char[fieldSize];
				entryData->Buffer = entryBuffer;
				if (!entryBuffer)
				{
					abort = true;
					break;
				}
				fread(entryBuffer, 1, fieldSize, fileHandle);

				switch (entryType) {
					case FieldTypes::ShortValue:
					case FieldTypes::Unknown2:
						*(int16_t*)entryBuffer = SDL_SwapLE16(*(int16_t*)entryBuffer);
						break;
					case FieldTypes::ShortArray:
						for (int i = 0; i < fieldSize / 2; i++) {
							((int16_t*)entryBuffer)[i] = SDL_SwapLE16(((int16_t*)entryBuffer)[i]);
						}
						break;
					case FieldTypes::FloatArray:
						for (int i = 0; i < fieldSize / 4; i++) {
							((float*)entryBuffer)[i] = SDL_SwapFloatLE(((float*)entryBuffer)[i]);
						}
						break;
					case FieldTypes::Palette:
						for (int i = 0; i < fieldSize / 4; i++) {
							((uint32_t*)entryBuffer)[i] = SDL_SwapLE32(((uint32_t*)entryBuffer)[i]);
						}
						break;
				}
			}

			groupData->AddEntry(entryData);
		}

		datFile->Groups.push_back(groupData);
	}

	fclose(fileHandle);
	if (datFile->Groups.size() == header.NumberOfGroups)
	{
		datFile->Finalize();
		return datFile;
	}
	delete datFile;
	return nullptr;
}
