#pragma once

struct field_effect_type;
class TEdgeSegment;

class TEdgeBox
{
public:
	std::vector<TEdgeSegment*> EdgeList{};
	std::vector<field_effect_type*> FieldList{};
};
