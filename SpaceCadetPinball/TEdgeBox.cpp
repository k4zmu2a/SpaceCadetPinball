#include "pch.h"
#include "TEdgeBox.h"

#include "objlist_class.h"

TEdgeBox::TEdgeBox()
{
	EdgeList = new objlist_class(0, 4);
	FieldList = new objlist_class(0, 1);
}

TEdgeBox::~TEdgeBox()
{
	delete EdgeList;
	delete FieldList;
}
