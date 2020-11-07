#include "pch.h"
#include "pb.h"
#include "render.h"

TPinballTable* pb::MainTable = nullptr;

void pb::reset_table()
{
	if (MainTable)
		MainTable->Message(1024, 0.0);
}


void pb::firsttime_setup()
{
	render::blit = 0;
	render::update();
	render::blit = 1;
}

void pb::paint()
{
	render::paint();
}