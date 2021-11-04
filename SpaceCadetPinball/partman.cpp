#include "pch.h"
#include "partman.h"

#include "gdrv.h"
#include "GroupData.h"
#include "zdrv.h"

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#	if defined(__GNUC__) && defined(linux)
#		include <byteswap.h>
#		define scp_bswap64(x) __bswap_64(x)
#		define scp_bswap32(x) __bswap_32(x)
#		define scp_bswap16(x) __bswap_16(x)
#	endif //__GNUC__ && linux
#endif

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
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	header.FileSize = scp_bswap32(header.FileSize);
	header.NumberOfGroups = scp_bswap16(header.NumberOfGroups);
	header.SizeOfBody = scp_bswap32(header.SizeOfBody);
	header.Unknown = scp_bswap16(header.Unknown);
#endif
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
			size_t fieldSize;
			if(fixedSize >= 0) fieldSize = fixedSize;
			else {
				fieldSize = LRead<uint32_t>(fileHandle);
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
				fieldSize = scp_bswap32(fieldSize);
#endif //__BIG_ENDIAN
			}
			entryData->FieldSize = static_cast<int>(fieldSize);

			if (entryType == FieldTypes::Bitmap8bit)
			{
				fread(&bmpHeader, 1, sizeof(dat8BitBmpHeader), fileHandle);
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
				bmpHeader.Width = scp_bswap16(bmpHeader.Width);
				bmpHeader.Height = scp_bswap16(bmpHeader.Height);
				bmpHeader.XPosition = scp_bswap16(bmpHeader.XPosition);
				bmpHeader.YPosition = scp_bswap16(bmpHeader.YPosition);
				bmpHeader.Size = scp_bswap32(bmpHeader.Size);
#endif //__BIG_ENDIAN__
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
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
				zMapHeader.Width = scp_bswap16(zMapHeader.Width);
				zMapHeader.Height = scp_bswap16(zMapHeader.Height);
				zMapHeader.Stride = scp_bswap16(zMapHeader.Stride);
				zMapHeader.Unknown0 = scp_bswap32(zMapHeader.Unknown0);
				zMapHeader.Unknown1_0 = scp_bswap16(zMapHeader.Unknown1_0);
				zMapHeader.Unknown1_1 = scp_bswap16(zMapHeader.Unknown1_1);
#endif //__BIG_ENDIAN__
				auto length = fieldSize - sizeof(dat16BitBmpHeader);

				auto zMap = new zmap_header_type(zMapHeader.Width, zMapHeader.Height, zMapHeader.Stride);
				zMap->Resolution = zMapResolution;
				if (zMapHeader.Stride * zMapHeader.Height * 2u == length)
				{
					fread(zMap->ZPtr1, 1, length, fileHandle);
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
					int16_t * temporaryPointer = (int16_t *)zMap->ZPtr1;
					for(size_t temporaryCounter = 0; temporaryCounter < length/2; ++temporaryCounter) {
						temporaryPointer[temporaryCounter] = scp_bswap16(temporaryPointer[temporaryCounter]);
					}
#endif
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
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
				if(entryType == FieldTypes::ShortValue) {
					*((int16_t*)entryBuffer) = scp_bswap16(*((int16_t*)entryBuffer));
				}
				if(entryType == FieldTypes::Palette) {
					int32_t * temporaryPointer = (int32_t *)entryBuffer;
					for(int16_t pixelCounter = 0; pixelCounter < 256; ++pixelCounter) {
						temporaryPointer[pixelCounter] = scp_bswap32(temporaryPointer[pixelCounter]);
					}
				}
				if(entryType == FieldTypes::ShortArray) {
					int16_t * temporaryPointer = (int16_t *)entryBuffer;
					for(size_t temporaryCounter = 0; temporaryCounter < fieldSize/2; ++temporaryCounter) {
						temporaryPointer[temporaryCounter] = scp_bswap16(temporaryPointer[temporaryCounter]);
					}
				}
				if(entryType == FieldTypes::FloatArray) {
					int32_t * temporaryPointer = (int32_t *)entryBuffer;
					for(size_t temporaryCounter = 0; temporaryCounter < fieldSize/4; ++temporaryCounter) {
						temporaryPointer[temporaryCounter] = scp_bswap32(temporaryPointer[temporaryCounter]);
					}
				}
#endif
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
