#pragma once

struct zmap_header_type;
struct gdrv_bitmap8;


enum class FieldTypes : int16_t
{
	// One 16 bit signed integer
	ShortValue = 0,

	// Sprite bitmap, 8bpp, indexed color
	Bitmap8bit = 1,

	Unknown2 = 2,

	// Group name, char[]. Not all groups have names.
	GroupName = 3,

	Unknown4 = 4,

	// Palette, contains 256 RBGA 4-byte colors.
	Palette = 5,

	Unknown6 = 6,

	Unknown7 = 7,

	Unknown8 = 8,

	// String, char[]
	String = 9,

	// Array of 16 bit signed integers
	ShortArray = 10,

	// Array of 32 bit floats
	FloatArray = 11,

	// Sprite depth map, 16bpp, unsigned
	Bitmap16bit = 12,
};

struct EntryData
{
	EntryData() = default;

	EntryData(FieldTypes entryType, char* buffer): EntryType(entryType), FieldSize(-1), Buffer(buffer)
	{
	}

	~EntryData();
	FieldTypes EntryType{};
	int FieldSize{};
	char* Buffer{};
};

class GroupData
{
public:
	int GroupId;
	std::string GroupName;

	GroupData(int groupId);
	void AddEntry(EntryData* entry);
	void FinalizeGroup();
	const std::vector<EntryData*>& GetEntries() const { return Entries; }
	const EntryData* GetEntry(size_t index) const { return Entries[index]; }
	size_t EntryCount() const { return Entries.size(); }
	void ReserveEntries(size_t count) { Entries.reserve(count); }
	gdrv_bitmap8* GetBitmap(int resolution) const;
	zmap_header_type* GetZMap(int resolution) const;

private:
	std::vector<EntryData*> Entries;
	gdrv_bitmap8* Bitmaps[3]{};
	zmap_header_type* ZMaps[3]{};
	bool NeedsSort = false;

	static void SplitSplicedBitmap(const gdrv_bitmap8& srcBmp, gdrv_bitmap8& bmp, zmap_header_type& zMap);

	void SetBitmap(gdrv_bitmap8* bmp);
	void SetZMap(zmap_header_type* zMap);
};


class DatFile
{
public:
	std::string AppName;
	std::string Description;
	std::vector<GroupData*> Groups;

	~DatFile();
	char* field_nth(int groupIndex, FieldTypes targetEntryType, int skipFirstN);
	char* field(int groupIndex, FieldTypes entryType);
	int field_size_nth(int groupIndex, FieldTypes targetEntryType, int skipFirstN);
	int field_size(int groupIndex, FieldTypes targetEntryType);
	int record_labeled(LPCSTR targetGroupName);
	char* field_labeled(LPCSTR lpString, FieldTypes fieldType);
	gdrv_bitmap8* GetBitmap(int groupIndex);
	zmap_header_type* GetZMap(int groupIndex);
};
