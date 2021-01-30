#pragma once
#include "objlist_class.h"


struct field_effect_type;
class TEdgeSegment;

class TEdgeBox
{
public:
	TEdgeBox();
	~TEdgeBox();

	objlist_class<TEdgeSegment>* EdgeList;
	objlist_class<field_effect_type>* FieldList;
};

