#pragma once

#include <objbase.h>  
#include <msxml6.h>
#include "dictionary.h"
#include <string>

using std::string;

// Helper function to create a VT_BSTR variant from a null terminated string. 
HRESULT VariantFromString(PCWSTR wszValue, VARIANT & Variant);

// Helper function to create a DOM instance. 
HRESULT CreateAndInitDOM(IXMLDOMDocument * *ppDoc);

dictionary* loadDOM(PCWSTR location);
dictionary* createDictionary(IXMLDOMDocument* theDoc);