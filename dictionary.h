#pragma once

#include <comdef.h>
 
class dictionary
{
private:
    int size;
    BSTR** mapping;

public:
    dictionary(int size);
    ~dictionary();
    void put(BSTR key, BSTR value);
    void print();
    BSTR get(BSTR key);
};