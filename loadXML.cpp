#include <msxml6.h>
#include "dictionary.h"
#include "loadXML.h"
#include <comdef.h>

// Macro that calls a COM method returning HRESULT value.
#define CHK_HR(stmt)        do { hr=(stmt); if (FAILED(hr)) goto CleanUp; } while(0)
// Macro to verify memory allcation.
#define CHK_ALLOC(p)        do { if (!(p)) { hr = E_OUTOFMEMORY; goto CleanUp; } } while(0)
// Macro that releases a COM object if not NULL.
#define SAFE_RELEASE(p)     do { if ((p)) { (p)->Release(); (p) = NULL; } } while(0)

dictionary* createDictionary(IXMLDOMDocument * theDoc);

// Helper function to create a VT_BSTR variant from a null terminated string. 
HRESULT VariantFromString(PCWSTR wszValue, VARIANT& Variant)
{
    HRESULT hr = S_OK;
    BSTR bstr = SysAllocString(wszValue);
    CHK_ALLOC(bstr);

    V_VT(&Variant) = VT_BSTR;
    V_BSTR(&Variant) = bstr;

CleanUp:
    return hr;
}

// Helper function to create a DOM instance. 
HRESULT CreateAndInitDOM(IXMLDOMDocument** ppDoc)
{
    HRESULT hr = CoCreateInstance(__uuidof(DOMDocument60), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(ppDoc));
    if (SUCCEEDED(hr))
    {
        // these methods should not fail so don't inspect result
        (*ppDoc)->put_async(VARIANT_FALSE);
        (*ppDoc)->put_validateOnParse(VARIANT_FALSE);
        (*ppDoc)->put_resolveExternals(VARIANT_FALSE);
    }
    return hr;
}

dictionary* loadDOM(PCWSTR location)
{
    dictionary* conversionTable = NULL;
    HRESULT hr = S_OK;
    IXMLDOMDocument* pXMLDom = NULL;
    IXMLDOMParseError* pXMLErr = NULL;

    BSTR bstrXML = NULL;
    BSTR bstrErr = NULL;
    VARIANT_BOOL varStatus;
    VARIANT varFileName;
    VariantInit(&varFileName);

    CHK_HR(CreateAndInitDOM(&pXMLDom));

    // XML file name to load
    CHK_HR(VariantFromString(location, varFileName));
    CHK_HR(pXMLDom->load(varFileName, &varStatus));
    if (varStatus == VARIANT_TRUE)
    {
        //CHK_HR(pXMLDom->get_xml(&bstrXML));
        printf("XML DOM loaded from %S:\n", location);
        conversionTable = createDictionary(pXMLDom);
    }
    else
    {
        // Failed to load xml, get last parsing error
        CHK_HR(pXMLDom->get_parseError(&pXMLErr));
        CHK_HR(pXMLErr->get_reason(&bstrErr));
        printf("Failed to load DOM from %S. %S\n", location, bstrErr);
    }

    return conversionTable;

CleanUp:
    SAFE_RELEASE(pXMLDom);
    SAFE_RELEASE(pXMLErr);
    SysFreeString(bstrXML);
    SysFreeString(bstrErr);
    VariantClear(&varFileName);
}

dictionary* createDictionary(IXMLDOMDocument* theDoc)
{
    if (theDoc == NULL)
        return NULL;

    IXMLDOMNodeList* allEntries = NULL;
    BSTR entryTagName = SysAllocString(L"Entry");
    BSTR currentValue = SysAllocString(L"value");;
    BSTR currentDefinition = SysAllocString(L"definition");
    BSTR value = NULL;
    BSTR definition = NULL;
    theDoc->getElementsByTagName(entryTagName, &allEntries);
    // going through the list
    long index = 0;
    IXMLDOMNode* currentItem = NULL;
    IXMLDOMNode* currentAttribute = NULL;
    IXMLDOMNamedNodeMap* attributes = NULL;
    long listLength = NULL;
    allEntries->get_length(&listLength);
    dictionary* returnMe = new dictionary((int)listLength);

    for (long index = 0; index < listLength; index++)
    {
        allEntries->get_item(index, &currentItem);
        currentItem->get_attributes(&attributes);

        attributes->getNamedItem(currentValue, &currentAttribute);
        currentAttribute->get_text(&value);
        attributes->getNamedItem(currentDefinition, &currentAttribute);
        currentAttribute->get_text(&definition);
        returnMe->put(value, definition);
    }
    
    // deleting what I used
    SysFreeString(entryTagName);
    SysFreeString(currentValue);
    SysFreeString(currentDefinition);
    currentItem->Release();
    currentAttribute->Release();
    attributes->Release();
    allEntries->Release();
    //SysFreeString(value);
    //SysFreeString(definition);

    return returnMe;
}