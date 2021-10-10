#include "pch.h"

#include "GroupData.h"

#include "EmbeddedData.h"
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

void DatFile::Finalize()
{
	if (!pb::FullTiltMode)
	{
		int groupIndex = record_labeled("pbmsg_ft");
		assertm(groupIndex < 0, "DatFile: pbmsg_ft is already in .dat");

		// Load 3DPB font into dat to simplify pipeline
		auto rcData = reinterpret_cast<MsgFont*>(ImFontAtlas::DecompressCompressedBase85Data(
			EmbeddedData::PB_MSGFT_bin_compressed_data_base85));
		AddMsgFont(rcData, "pbmsg_ft");
		IM_FREE(rcData);

		// PINBALL2.MID is an alternative font provided in 3DPB data
		/*auto file = pinball::make_path_name("PINBALL2.MID");
		auto fileHandle = fopen(file.c_str(), "rb");
		fseek(fileHandle, 0, SEEK_END);
		auto fileSize = static_cast<uint32_t>(ftell(fileHandle));
		auto rcData = reinterpret_cast<MsgFont*>(new uint8_t[fileSize]);
		fseek(fileHandle, 0, SEEK_SET);
		fread(rcData, 1, fileSize, fileHandle);
		fclose(fileHandle);
		AddMsgFont(rcData, "pbmsg_ft");
		delete[] rcData;*/
	}

	for (auto group : Groups)
	{
		group->FinalizeGroup();
	}
}

void DatFile::AddMsgFont(MsgFont* font, const std::string& fontName)
{
	auto groupId = Groups.back()->GroupId + 1;
	auto ptrToData = reinterpret_cast<char*>(font->Data);
	for (auto charInd = 32; charInd < 128; charInd++, groupId++)
	{
		auto curChar = reinterpret_cast<MsgFontChar*>(ptrToData);
		assertm(curChar->Width == font->CharWidths[charInd], "Score: mismatched font width");
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
