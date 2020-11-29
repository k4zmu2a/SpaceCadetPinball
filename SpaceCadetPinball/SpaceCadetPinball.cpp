// SpaceCadetPinball.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

#include <iostream>
#include "objlist_class.h"
#include "partman.h"
#include  "DatParser.h"
#include "gdrv.h"
#include "loader.h"
#include "pb.h"
#include "pinball.h"
#include "score.h"
#include "TPinballTable.h"
#include "TTextBox.h"

int main()
{
	std::cout << "Hello World!\n";
	{
		// Testing with UI
	/*	lstrcpyA(pinball::DatFileName, "PINBALL.DAT");
		pinball::hinst = GetModuleHandleA(nullptr);
		char cmdLine[1]{};
		pb::init();
		WinMain(pinball::hinst, 0, cmdLine, 10);*/
	}

	gdrv::init(0, 0);
	auto dib = gdrv::DibCreate(8, 1, 1);
	gdrv::DibSetUsage(dib, 0, 1);

	objlist_class d = objlist_class(2, 4);
	for (int i = 0; i < 100; i++)
	{
		d.Add((void*)i);
	}
	d.Delete((void*)3);

	auto xx = sizeof(datFileHeader);

	lstrcpyA(pinball::DatFileName, "PINBALL.DAT");
	pb::init();
	auto datFile = pb::record_table;

	assert(partman::field_size_nth(datFile, 0, datFieldTypes::String, 0) == 43);
	assert(partman::field_size_nth(datFile, 2, datFieldTypes::Palette, 0) == 1024);
	assert(partman::field_size_nth(datFile, 101, datFieldTypes::FloatArray, 4) == 32);

	assert(strcmp(partman::field(datFile, 0, datFieldTypes::String), "3D-Pinball:  Copyright 1994, Cinematronics") == 0);
	assert(strcmp(partman::field(datFile, 540, datFieldTypes::GroupName), "table_objects") == 0);

	assert(partman::record_labeled(datFile, "background") == 2);
	assert(partman::record_labeled(datFile, "a_bump1") == 372);

	assert(memcmp(partman::field_labeled(datFile, "table_size", datFieldTypes::ShortArray), new short[2]{ 600, 416 }, 2 * 2) == 0);

	//loader::error(25, 26);
	loader::get_sound_id(18);
	visualStruct visual1{};
	loader::material(96, &visual1);
	loader::query_visual(283, 0, &visual1);
	visualKickerStruct kicker1{};
	loader::kicker(509, &kicker1);

	auto score1 = score::create("score1", nullptr);

	auto pinballTable = pb::MainTable;
	//pinballTable->find_component(1);

	for (int i = 0; i < 190; i++)
	{
		auto rsc = pinball::get_rc_string(i, 0);
		if (rsc)
			printf_s("%d:\t%s\n", i, rsc);
	}
	
	//DatParser::Parse(dataFileName);
	std::cout << "Goodby World!\n";
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
