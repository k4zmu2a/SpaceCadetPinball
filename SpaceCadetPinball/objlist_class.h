#pragma once

struct  objlist_struct1
{
	int Size;
	int Count;
	void* Array[1];
};

class objlist_class
{
public:
	objlist_class(int SizeInt, int growSize);
	~objlist_class();
	void Add(void* value);
	void Grow();
	int Delete(void* value);
	void* Get(int index);
	int Count() const { return !ListPtr ? 0 : ListPtr->Count; }
	int Size() const { return !ListPtr ? 0 : ListPtr->Size; }	
private:
	objlist_struct1* ListPtr;
	int GrowSize;
	static objlist_struct1* objlist_new(int sizeInt);
	static int objlist_add_object(objlist_struct1* ptrToStruct, void* value);
	static objlist_struct1* objlist_grow(objlist_struct1* ptrToStruct, int growSize);
	static int objlist_delete_object(objlist_struct1* ptrToStruct, void* value);
};
