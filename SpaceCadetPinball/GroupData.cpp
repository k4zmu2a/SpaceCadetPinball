#include "pch.h"

#include "GroupData.h"

#include "fullscrn.h"
#include "gdrv.h"
#include "memory.h"
#include "zdrv.h"


EntryData::~EntryData()
{
	if (Buffer)
	{
		if (EntryType == FieldTypes::Bitmap8bit)
			gdrv::destroy_bitmap(reinterpret_cast<gdrv_bitmap8*>(Buffer));
		else if (EntryType == FieldTypes::Bitmap16bit)
			zdrv::destroy_zmap(reinterpret_cast<zmap_header_type*>(Buffer));
		memory::free(Buffer);
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
				auto bmp = memory::allocate<gdrv_bitmap8>();
				auto zMap = memory::allocate<zmap_header_type>();
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

	gdrv::create_bitmap(&bmp, srcBmp.Width, srcBmp.Height, srcBmp.Width);
	std::memset(bmp.IndexedBmpPtr, 0xff, bmp.Stride * bmp.Height);
	bmp.XPosition = srcBmp.XPosition;
	bmp.YPosition = srcBmp.YPosition;
	bmp.Resolution = srcBmp.Resolution;

	zdrv::create_zmap(&zMap, srcBmp.Width, srcBmp.Height, srcBmp.Width);
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

		int index;
		for (index = 0; index < targetLength; index++)
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
