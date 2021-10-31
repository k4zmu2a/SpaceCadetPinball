#include "pch.h"

#include "GroupData.h"

#include "fullscrn.h"
#include "gdrv.h"
#include "pb.h"
#include "pinball.h"
#include "zdrv.h"


EntryData::~EntryData()
{
	if (Buffer)
	{
		if (EntryType == FieldTypes::Bitmap8bit)
		{
			delete reinterpret_cast<gdrv_bitmap8*>(Buffer);
		}
		else if (EntryType == FieldTypes::Bitmap16bit)
		{
			delete reinterpret_cast<zmap_header_type*>(Buffer);
		}
		else
			delete[] Buffer;
	}
}


GroupData::GroupData(int groupId)
{
	GroupId = groupId;
}

void GroupData::AddEntry(EntryData* entry)
{
	auto addEntry = true;
	switch (entry->EntryType)
	{
	case FieldTypes::GroupName:
		GroupName = entry->Buffer;
		break;
	case FieldTypes::Bitmap8bit:
		{
			auto srcBmp = reinterpret_cast<gdrv_bitmap8*>(entry->Buffer);
			if (srcBmp->BitmapType == BitmapTypes::Spliced)
			{
				// Get rid of spliced bitmap early on, to simplify render pipeline
				auto bmp = new gdrv_bitmap8(srcBmp->Width, srcBmp->Height, srcBmp->Width);
				auto zMap = new zmap_header_type(srcBmp->Width, srcBmp->Height, srcBmp->Width);
				SplitSplicedBitmap(*srcBmp, *bmp, *zMap);

				NeedsSort = true;
				addEntry = false;
				AddEntry(new EntryData(FieldTypes::Bitmap8bit, reinterpret_cast<char*>(bmp)));
				AddEntry(new EntryData(FieldTypes::Bitmap16bit, reinterpret_cast<char*>(zMap)));
				delete entry;
			}
			else
			{
				SetBitmap(srcBmp);
			}
			break;
		}
	case FieldTypes::Bitmap16bit:
		{
			SetZMap(reinterpret_cast<zmap_header_type*>(entry->Buffer));
			break;
		}
	default: break;
	}

	if (addEntry)
		Entries.push_back(entry);
}

void GroupData::FinalizeGroup()
{
	if (NeedsSort)
	{
		// Entries within a group are sorted by EntryType, in ascending order.
		// Dat files follow this rule, zMaps inserted in the middle break it.
		NeedsSort = false;
		std::sort(Entries.begin(), Entries.end(), [](const EntryData* lhs, const EntryData* rhs)
		{
			return lhs->EntryType < rhs->EntryType;
		});
		Entries.shrink_to_fit();
	}
}

gdrv_bitmap8* GroupData::GetBitmap(int resolution) const
{
	return Bitmaps[resolution];
}

zmap_header_type* GroupData::GetZMap(int resolution) const
{
	return ZMaps[resolution];
}

void GroupData::SplitSplicedBitmap(const gdrv_bitmap8& srcBmp, gdrv_bitmap8& bmp, zmap_header_type& zMap)
{
	assertm(srcBmp.BitmapType == BitmapTypes::Spliced, "GroupData: wrong bitmap type");

	std::memset(bmp.IndexedBmpPtr, 0xff, bmp.Stride * bmp.Height);
	bmp.XPosition = srcBmp.XPosition;
	bmp.YPosition = srcBmp.YPosition;
	bmp.Resolution = srcBmp.Resolution;

	zdrv::fill(&zMap, zMap.Width, zMap.Height, 0, 0, 0xFFFF);
	zMap.Resolution = srcBmp.Resolution;

	auto tableWidth = fullscrn::resolution_array[srcBmp.Resolution].TableWidth;
	auto src = reinterpret_cast<uint16_t*>(srcBmp.IndexedBmpPtr);
	auto srcChar = reinterpret_cast<char**>(&src);
	for (int dstInd = 0;;)
	{
		auto stride = static_cast<int16_t>(*src++);
		if (stride < 0)
			break;

		// Stride is in terms of dst stride, hardcoded to match vScreen width in current resolution
		if (stride > bmp.Width)
		{
			stride += bmp.Width - tableWidth;
			assertm(stride >= 0, "Spliced bitmap: negative computed stride");
		}

		dstInd += stride;
		for (auto count = *src++; count; count--)
		{
			auto depth = *src++;
			bmp.IndexedBmpPtr[dstInd] = **srcChar;
			zMap.ZPtr1[dstInd] = depth;

			(*srcChar)++;
			dstInd++;
		}
	}
}

void GroupData::SetBitmap(gdrv_bitmap8* bmp)
{
	assertm(Bitmaps[bmp->Resolution] == nullptr, "GroupData: bitmap override");
	Bitmaps[bmp->Resolution] = bmp;

	auto zMap = ZMaps[bmp->Resolution];
	if (zMap)
	{
		assertm(bmp->Width == zMap->Width && bmp->Height == zMap->Height,
		        "GroupData: mismatched bitmap/zMap dimensions");
	}
}

void GroupData::SetZMap(zmap_header_type* zMap)
{
	// Flip zMap to match with flipped non-indexed bitmaps
	zdrv::FlipZMapHorizontally(*zMap);

	assertm(ZMaps[zMap->Resolution] == nullptr, "GroupData: zMap override");
	ZMaps[zMap->Resolution] = zMap;

	auto bmp = Bitmaps[zMap->Resolution];
	if (bmp)
	{
		assertm(bmp->Width == zMap->Width && bmp->Height == zMap->Height,
		        "GroupData: mismatched bitmap/zMap dimensions");
	}
}


DatFile::~DatFile()
{
	for (auto group : Groups)
	{
		if (!group)
			continue;

		for (const auto entry : group->GetEntries())
		{
			delete entry;
		}
		delete group;
	}
}

char* DatFile::field(int groupIndex, FieldTypes targetEntryType)
{
	assertm(targetEntryType != FieldTypes::Bitmap8bit && targetEntryType != FieldTypes::Bitmap16bit,
	        "partman: Use specific get for bitmaps");

	auto group = Groups[groupIndex];
	for (const auto entry : group->GetEntries())
	{
		if (entry->EntryType == targetEntryType)
		{
			return entry->Buffer;
		}
		if (entry->EntryType > targetEntryType)
			break;
	}
	return nullptr;
}


char* DatFile::field_nth(int groupIndex, FieldTypes targetEntryType, int skipFirstN)
{
	assertm(targetEntryType != FieldTypes::Bitmap8bit && targetEntryType != FieldTypes::Bitmap16bit,
	        "partman: Use specific get for bitmaps");

	auto group = Groups[groupIndex];
	auto skipCount = 0;
	for (const auto entry : group->GetEntries())
	{
		if (entry->EntryType > targetEntryType)
			break;
		if (entry->EntryType == targetEntryType)
			if (skipCount++ == skipFirstN)
				return entry->Buffer;
	}
	return nullptr;
}

int DatFile::field_size_nth(int groupIndex, FieldTypes targetEntryType, int skipFirstN)
{
	auto group = Groups[groupIndex];
	auto skipCount = 0;
	for (const auto entry : group->GetEntries())
	{
		if (entry->EntryType > targetEntryType)
			return 0;
		if (entry->EntryType == targetEntryType)
			if (skipCount++ == skipFirstN)
				return entry->FieldSize;
	}
	return 0;
}

int DatFile::field_size(int groupIndex, FieldTypes targetEntryType)
{
	return field_size_nth(groupIndex, targetEntryType, 0);
}

int DatFile::record_labeled(LPCSTR targetGroupName)
{
	auto targetLength = strlen(targetGroupName);
	for (int groupIndex = Groups.size() - 1; groupIndex >= 0; --groupIndex)
	{
		auto groupName = field(groupIndex, FieldTypes::GroupName);
		if (!groupName)
			continue;

		auto index = 0u;
		for (; index < targetLength; index++)
			if (targetGroupName[index] != groupName[index])
				break;
		if (index == targetLength && !targetGroupName[index] && !groupName[index])
			return groupIndex;
	}

	return -1;
}

char* DatFile::field_labeled(LPCSTR lpString, FieldTypes fieldType)
{
	auto groupIndex = record_labeled(lpString);
	return groupIndex < 0 ? nullptr : field(groupIndex, fieldType);
}

gdrv_bitmap8* DatFile::GetBitmap(int groupIndex)
{
	auto group = Groups[groupIndex];
	return group->GetBitmap(fullscrn::GetResolution());
}

zmap_header_type* DatFile::GetZMap(int groupIndex)
{
	auto group = Groups[groupIndex];
	return group->GetZMap(fullscrn::GetResolution());
}

MsgFont *DatFile::ReadPEMsgFontResource(const std::string peName)
{
	auto file = pinball::make_path_name(peName);
	auto fileHandle = fopen(file.c_str(), "rb");

	if (fileHandle == nullptr)
		return nullptr;

	fseek(fileHandle, 0x3C, SEEK_SET);
	constexpr uint8_t peHeaderSize = 0x18;
	uint32_t peHeaderOffset;
	fread(&peHeaderOffset, sizeof(uint32_t), 1, fileHandle);

	fseek(fileHandle, peHeaderOffset + 0x6, SEEK_SET);
	uint16_t numberOfSections;
	fread(&numberOfSections, sizeof(uint16_t), 1, fileHandle);

	if (numberOfSections != 3)
	{
		fclose(fileHandle);
		return nullptr;
	}

	fseek(fileHandle, peHeaderOffset + 0x14, SEEK_SET);
	uint16_t sizeOfOptionalHeader;
	fread(&sizeOfOptionalHeader, sizeof(uint16_t), 1, fileHandle);

	constexpr uint8_t sizeOfSectionHeader = 0x28;
	uint32_t sectionHeadersOffset = peHeaderOffset + peHeaderSize + sizeOfOptionalHeader;
	uint32_t thirdSectionHeaderOffset = sectionHeadersOffset + sizeOfSectionHeader * 2;
	fseek(fileHandle, thirdSectionHeaderOffset, SEEK_SET);

	char sectionName[0x8] = {};
	fread(sectionName, sizeof(char), 0x8, fileHandle);

	if (strcmp(sectionName, ".rsrc\0") != 0)
	{
		fclose(fileHandle);
		return nullptr;
	}

	fseek(fileHandle, thirdSectionHeaderOffset + 0xc, SEEK_SET);
	uint32_t rsrcSectionVirtualAddress;
	fread(&rsrcSectionVirtualAddress, sizeof(uint32_t), 1, fileHandle);

	fseek(fileHandle, thirdSectionHeaderOffset + 0x14, SEEK_SET);
	uint32_t rsrcSectionDataOffset;
	fread(&rsrcSectionDataOffset, sizeof(uint32_t), 1, fileHandle);

	fseek(fileHandle, rsrcSectionDataOffset + 0xe, SEEK_SET);
	uint16_t numberOfIDEntries;
	fread(&numberOfIDEntries, sizeof(uint16_t), 1, fileHandle);

	if (numberOfIDEntries == 0)
	{
		fclose(fileHandle);
		return nullptr;
	}

	uint32_t subdirectoryOffset = 0;

	for (uint16_t e = 0; e < numberOfIDEntries; e++)
	{
		fseek(fileHandle, rsrcSectionDataOffset + 0x10 + (0x8 * e), SEEK_SET);
		int32_t entryIntegerID;
		fread(&entryIntegerID, sizeof(int32_t), 1, fileHandle);

		if (entryIntegerID == 0xa) // RCDATA
		{
			fread(&subdirectoryOffset, sizeof(uint32_t), 1, fileHandle);
			subdirectoryOffset &= 0x7fffffff;
			subdirectoryOffset += rsrcSectionDataOffset;
			break;
		}
	}

	if (subdirectoryOffset == 0)
	{
		fclose(fileHandle);
		return nullptr;
	}

	fseek(fileHandle, subdirectoryOffset + 0xc, SEEK_SET);
	uint16_t numberOfNameEntries;
	fread(&numberOfNameEntries, sizeof(uint16_t), 1, fileHandle);

	if (numberOfNameEntries == 0)
	{
		fclose(fileHandle);
		return nullptr;
	}

	fseek(fileHandle, subdirectoryOffset + 0x10, SEEK_SET);

	uint32_t nameOffset;
	fread(&nameOffset, sizeof(uint32_t), 1, fileHandle);
	nameOffset &= 0x7fffffff;
	nameOffset += rsrcSectionDataOffset;

	fread(&subdirectoryOffset, sizeof(uint32_t), 1, fileHandle);
	subdirectoryOffset &= 0x7fffffff;
	subdirectoryOffset += rsrcSectionDataOffset;

	fseek(fileHandle, nameOffset, SEEK_SET);
	uint16_t expectedName[9] = {0x08, 0x50, 0x42, 0x4d, 0x53, 0x47, 0x5f, 0x46, 0x54}; // PBMSG_FT
	uint16_t name[9];
	fread(name, sizeof(uint16_t), 9, fileHandle);

	for (int n = 0; n < 9; n++)
	{
		if (name[n] != expectedName[n])
		{
			fclose(fileHandle);
			return nullptr;
		}
	}

	fseek(fileHandle, subdirectoryOffset + 0xe, SEEK_SET);
	fread(&numberOfIDEntries, sizeof(uint16_t), 1, fileHandle);

	if (numberOfIDEntries == 0)
	{
		fclose(fileHandle);
		return nullptr;
	}

	// If the user has some pinball.exe in a language other than English,
	// or multiple languages, there would be one IDEntry for each language here.

	/*fseek(fileHandle, subdirectoryOffset + 0x10, SEEK_SET);
	uint32_t languageID;
	fread(&languageID, sizeof(uint32_t), 1, fileHandle);*/

	fseek(fileHandle, subdirectoryOffset + 0x14, SEEK_SET);
	uint32_t dataEntryOffset;
	fread(&dataEntryOffset, sizeof(uint32_t), 1, fileHandle);
	dataEntryOffset &= 0x7fffffff;
	dataEntryOffset += rsrcSectionDataOffset;

	fseek(fileHandle, dataEntryOffset, SEEK_SET);
	uint32_t dataOffset, dataSize;
	fread(&dataOffset, sizeof(uint32_t), 1, fileHandle);
	fread(&dataSize, sizeof(uint32_t), 1, fileHandle);

	fseek(fileHandle, (rsrcSectionDataOffset - rsrcSectionVirtualAddress) + dataOffset, SEEK_SET);
	MsgFont *msgFont = reinterpret_cast<MsgFont *>(new uint8_t[dataSize]);
	fread(msgFont, 1, dataSize, fileHandle);

	fclose(fileHandle);

	return msgFont;
}

void DatFile::Finalize()
{
	if (!pb::FullTiltMode)
	{
		int groupIndex = record_labeled("pbmsg_ft");
		assertm(groupIndex < 0, "DatFile: pbmsg_ft is already in .dat");

		MsgFont *rcData = ReadPEMsgFontResource("pinball.exe");

		if (rcData != nullptr)
		{
			AddMsgFont(rcData, "pbmsg_ft", false);
		}
		else
		{
			// PINBALL2.MID is an alternative font provided in 3DPB data
			// Scaled down because it is too large for top text box

			auto file = pinball::make_path_name("PINBALL2.MID");
			auto fileHandle = fopen(file.c_str(), "rb");
			fseek(fileHandle, 0, SEEK_END);
			auto fileSize = static_cast<uint32_t>(ftell(fileHandle));
			rcData = reinterpret_cast<MsgFont *>(new uint8_t[fileSize]);
			fseek(fileHandle, 0, SEEK_SET);
			fread(rcData, 1, fileSize, fileHandle);
			fclose(fileHandle);

			auto groupId = Groups.back()->GroupId + 1u;
			AddMsgFont(rcData, "pbmsg_ft", true);

			for (auto i = groupId; i < Groups.size(); i++)
				Groups[i]->GetBitmap(0)->ScaleIndexed(0.84f, 0.84f);
		}

		delete[] rcData;
	}

	for (auto group : Groups)
	{
		group->FinalizeGroup();
	}
}

void DatFile::AddMsgFont(MsgFont* font, const std::string& fontName, const bool changePaletteIndices)
{
	auto groupId = Groups.back()->GroupId + 1;
	auto ptrToData = reinterpret_cast<char*>(font->Data);
	for (auto charInd = 32; charInd < 128; charInd++, groupId++)
	{
		auto curChar = reinterpret_cast<MsgFontChar*>(ptrToData);
		assertm(curChar->Width == font->CharWidths[charInd], "Score: mismatched font width");

		if (changePaletteIndices)
		{
			// The alternate font in PINBALL2.MID is hard to read due to having dark colors.
			// Modify the pixels so it matches the other font.

			for (int i = 0; i < curChar->Width * font->Height; i++)
			{
				if (curChar->Data[i] == 0x50) curChar->Data[i] = 0x30;
				if (curChar->Data[i] == 0x69) curChar->Data[i] = 0x64;
			}
		}

		ptrToData += curChar->Width * font->Height + 1;

		auto bmp = new gdrv_bitmap8(curChar->Width, font->Height, true);
		auto srcPtr = curChar->Data;
		auto dstPtr = &bmp->IndexedBmpPtr[bmp->Stride * (bmp->Height - 1)];
		for (auto y = 0; y < font->Height; ++y)
		{
			memcpy(dstPtr, srcPtr, curChar->Width);
			srcPtr += curChar->Width;
			dstPtr -= bmp->Stride;
		}

		auto group = new GroupData(groupId);
		group->AddEntry(new EntryData(FieldTypes::Bitmap8bit, reinterpret_cast<char*>(bmp)));
		if (charInd == 32)
		{
			// First font group holds font name and gap width
			auto groupName = new char[fontName.length() + 1];
			strcpy(groupName, fontName.c_str());
			group->AddEntry(new EntryData(FieldTypes::GroupName, groupName));

			auto gaps = new char[2];
			*reinterpret_cast<int16_t*>(gaps) = font->GapWidth;
			group->AddEntry(new EntryData(FieldTypes::ShortArray, gaps));
		}
		else
		{
			auto groupName = new char[30];
			sprintf(groupName, "char %d='%c'", charInd, charInd);
			group->AddEntry(new EntryData(FieldTypes::GroupName, groupName));
		}

		Groups.push_back(group);
	}
}
