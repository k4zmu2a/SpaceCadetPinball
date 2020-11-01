#include "pch.h"
#include "DatParser.h"



typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;

FILE* ff; unsigned char* fdat; int fsiz;
char tbuf[256];
int ngrp;

void err(int n)
{
	printf("Error %i\n", n);
	exit(n);
}

void cerr(int c, int n)
{
	if (c) err(n);
}

void safeprint(char* s, int n)
{
	int i;
	for (i = 0; i < n; i++)
	{
		if (!s[i]) break;
		fputc(s[i], stdout);
	}
}

void printhexbytestr(uchar* s, int n)
{
	int i;
	for (i = 0; i < n; i++)
		printf("%02X", s[i]);
}

void printshortstr(short* s, int n)
{
	int i;
	for (i = 0; i < n; i++)
		printf(" %i", s[i]);
}

void printfloatstr(float* s, int n)
{
	int i;
	for (i = 0; i < n; i++)
		printf(" %f", s[i]);
}

int main2(int argc, const char* argv[])
{
	int g, en, es, n, et; unsigned char* p;

	printf("pbwdlist - 3D Pinball for Windows DAT file listing program\nby AdrienTD\n\n");
	if (argc < 2) { printf("Usage: pbwdlist FILE.DAT\n"); return 1; }

	FILE *ff;
	 fopen_s(&ff,argv[1], "rb");
	//ff = fopen(argv[1], "rb");
	cerr(!ff, -1);
	printf("File: %s\n\n", argv[1]);
	fseek(ff, 0, SEEK_END);
	fsiz = ftell(ff);
	fdat = (unsigned char*)malloc(fsiz);
	cerr(!fdat, -2);
	fseek(ff, 0, SEEK_SET);
	fread(fdat, fsiz, 1, ff);
	fclose(ff);

	printf("-- Header --");
	printf("\nFile signature:\t"); safeprint((char*)fdat, 21);
	printf("\nApp name:\t"); safeprint((char*)fdat + 0x15, 50);
	printf("\nDescription:\t"); safeprint((char*)fdat + 0x47, 100);
	printf("\nFile size:\t%i", *((int*)(fdat + 0xAB)));
	printf("\nNum. groups:\t%i", ngrp = *((short*)(fdat + 0xAF)));
	printf("\nSize of body:\t%i", *((int*)(fdat + 0xB1)));
	printf("\nUnknown value:\t%i", *((short*)(fdat + 0xB5)));

	printf("\n\n-- Body --");
	p = fdat + 0xB7;
	for (g = 0; g < ngrp; g++)
	{
		n = *(p++);
		printf("\nGroup %i:\tnum entries: %i, location: 0x%X\n", g, n, p - fdat - 1);
		for (en = 0; en < n; en++)
		{
			et = *(p++);
			if (et)
			{
				es = *((int*)p); p += 4;
				printf("\t\t- type: %i, size: %i\n", et, es);
				switch (et)
				{
				case 1: // Bitmap
					printf("\t\t  Bitmap, width: %i, height: %i\n", *(ushort*)(p + 1), *(ushort*)(p + 3));
					break;
				case 3:	// Group name
					printf("\t\t  Group name: ");
					safeprint((char*)p, es);
					printf("\n"); break;
				case 5: // Palette
					printf("\t\t  Palette\n"); break;
				case 9:	// String
					printf("\t\t  String: ");
					safeprint((char*)p, es);
					printf("\n"); break;
				case 10:
					//printf("\t\t  Content: ");
					//printhexbytestr(p, es);
					printf("\t\t  Shorts:");
					printshortstr((short*)p, es / 2);
					printf("\n"); break;
				case 11:
					printf("\t\t  Floats:");
					printfloatstr((float*)p, es / 4);
					printf("\n"); break;
				case 12:
					printf("\t\t  Special bitmap\n"); break;
				default:
					printf("\t\t  Unknown!\n"); break;
				}
				p += es;
			}
			else
			{
				es = *((short*)p); p += 2;
				printf("\t\t- type: %i, value: %i\n", et, es);
			}
		}
	}

	free(fdat);
}



void DatParser::Parse(const char* file)
{
	main2(2, new const char* [2] {0, file});
}



