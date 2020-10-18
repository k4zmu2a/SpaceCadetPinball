#pragma once

struct __declspec(align(4)) objlist_struct1
{
	int Size;
	int Count;
	int Array[1];
};


class objlist_class
{
public:
	objlist_class(int SizeInt, int growSize);
	~objlist_class();
	void Add(void* value);
	void Grow();
	int Delete(int value);

private:
	objlist_struct1* ListPtr;
	int GrowSize;
	objlist_struct1* objlist_new(int sizeInt);
	int objlist_add_object(objlist_struct1 *ptrToStruct, int value);
	objlist_struct1*  objlist_grow(objlist_struct1 *ptrToStruct, int growSize);
	int objlist_delete_object(objlist_struct1 *ptrToStruct, int value);
};