#include "dictionary.h"
#include <iostream>

using std::wprintf;

dictionary::dictionary(int size)
{
	this->size = size;
	mapping = new BSTR* [size];
	for (int i = 0; i < size; i++)
	{
		mapping[i] = new BSTR[2];
		mapping[i][0] = BSTR();
		mapping[i][1] = BSTR();
	}
}

dictionary::~dictionary()
{
	for (int i = 0; i < size; i++)
	{
		delete mapping[i];
	}
	delete mapping;
}

BSTR dictionary::get(BSTR key)
{
	for (int i = 0; i < size; i++)
	{
		if (0 == wcscmp(key, mapping[i][0]))
			return mapping[i][1];
	}
	return NULL;
}

void dictionary::put(BSTR key, BSTR value)
{
	int i = -1;
	for (int j = 0; j < size; j++)
	{
		if (SysStringLen(mapping[j][0]) == 0)
		{
			i = j;
			break;
		}
	}
	
	if (i == -1)
		wprintf(L"Dictionary is full");
	else
	{
		mapping[i][0] = key;
		mapping[i][1] = value;
	}
}

void dictionary::print()
{
	for (int i = 0; i < size; i++)
	{
		wprintf(mapping[i][0]);
		wprintf(L" ");
		wprintf(mapping[i][1]);
		wprintf(L"\n");
	}
}