// SpaceCadetPinball.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include "objlist_class.h"
#include "partman.h"
#include  "DatParser.h"

int main()
{
	std::cout << "Hello World!\n";

	objlist_class d = objlist_class(2, 4);
	for (int i = 0; i < 100; i++)
	{
		d.Add((void*)i);
	}
	d.Delete(3);

	auto xx = sizeof(datFileHeader);

	char dataFileName[300];
	partman::make_path_name(dataFileName, "PINBALL.DAT");
	auto datFile = partman::load_records(dataFileName);

	//DatParser::Parse(dataFileName);
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
