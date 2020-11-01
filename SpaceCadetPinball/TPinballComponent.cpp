#include "pch.h"
#include "TPinballComponent.h"
#include "loader.h"
#include "objlist_class.h"
#include "TZmapList.h"
#include "TPinballTable.h"

TPinballComponent::TPinballComponent(TPinballTable* table, int groupIndex, bool loadVisuals)
{
	visualStruct visual{}; // [esp+Ch] [ebp-6Ch]

	// this->VfTable = (int)&TPinballComponent::`vftable';
	this->Unknown2 = 0;
	this->UnknownBaseFlag1 = 0;
	this->UnknownBaseFlag2 = 0;
	this->PinballTable = table;
	this->Unknown7 = 0;
	this->List1Bitmap8 = nullptr;
	this->List2Bitmap16 = nullptr;
	if (table)
		table->ListP1->Add(this);
	if (groupIndex >= 0)
		this->GroupName = loader::query_name(groupIndex);
	if (loadVisuals && groupIndex >= 0)
	{
		int visualCount = loader::query_visual_states(groupIndex);
		for (int index = 0; index < visualCount; ++index)
		{
			loader::query_visual(groupIndex, index, &visual);
			if (visual.Bitmap8)
			{
				if (!this->List1Bitmap8)
					this->List1Bitmap8 = new TZmapList(visualCount, 4);
				if (this->List1Bitmap8)
					this->List1Bitmap8->Add(visual.Bitmap8);
			}
			if (visual.Bitmap16)
			{
				if (!this->List2Bitmap16)
					this->List2Bitmap16 = new TZmapList(visualCount, 4);
				if (this->List2Bitmap16)
					this->List2Bitmap16->Add(visual.Bitmap16);
			}
		}
		if (this->List2Bitmap16)
			int listVal0 = (int)this->List2Bitmap16->Get(0);
		if (this->List1Bitmap8)
		{
			/*listVal0_2 = (int*)this->List1Bitmap8->Get(0);
			v24 = *(int*)((char*)listVal0_2 + 29) - table->UnknownP49;
			v15 = 1;
			v25 = *(int*)((char*)listVal0_2 + 33) - table->UnknownP50;
			v26 = listVal0_2[3];
			v27 = listVal0_2[4];
			if (List1Bitmap8->Count() > 1)
			{
			    index = 12;
			    do
			    {
			        v16 = *(int**)((char*)&this->List1Bitmap8->ListPtr->Size + index);
			        v20 = *(int*)((char*)v16 + 29) - table->UnknownP49;
			        v21 = *(int*)((char*)v16 + 33) - table->UnknownP50;
			        v22 = v16[3];
			        v23 = v16[4];
			        enclosing_box(&v24, &v20, &v24);
			        index += 4;
			        ++v15;
			    } while (v15 < this->List1Bitmap8->ListPtr->Count);
			}
			v17 = this->List1Bitmap8->ListPtr->Array[0];
			this->Unknown7 = (int)render_create_sprite(
			    visualCount > 0,
			    this->List1Bitmap8->ListPtr->Array[0],
			    listVal0,
			    *(int*)(v17 + 29) - table->UnknownP49,
			    *(int*)(v17 + 33) - table->UnknownP50,
			    &v24);*/
		}
	}
	this->GroupIndex = groupIndex;
}
