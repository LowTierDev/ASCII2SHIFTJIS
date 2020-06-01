#include <iostream>
#include "computation.h"
#include "loadXML.h"
#include <fstream>
#include <comutil.h>

using std::cin;
using std::cout;
using std::size_t;
using std::string;
using std::ifstream;
using std::ofstream;
using std::wstring;
using _com_util::ConvertStringToBSTR;
using _com_util::ConvertBSTRToString;

// Goals:
// x    Open an XML file
// x    load conversion table from the file
// x    Open a file to convert
// x    go through a file and convert all the strings into the appropriate hex
// x    Arrange the hex in the right manner (4 byte words)
// o    save file as a regular file

BSTR Concat(BSTR a, BSTR b)
{
    auto lengthA = SysStringLen(a);
    auto lengthB = SysStringLen(b);

    auto result = SysAllocStringLen(NULL, lengthA + lengthB);

    memcpy(result, a, lengthA * sizeof(OLECHAR));
    memcpy(result + lengthA, b, lengthB * sizeof(OLECHAR));

    result[lengthA + lengthB] = 0;
    return result;
}

string segment(string line, size_t* currentPosition)
{
    string temp = string(1, line[*currentPosition]);
    (*currentPosition)++;

    while (line[*currentPosition] != '\\')
    {
        temp += string(1, line[*currentPosition]);
        (*currentPosition)++;
    }

    temp += string(1, line[*currentPosition]);
    return temp;
}


int main()
{
    string sourcePath, inputFile, outputFile;
    cout << "Please Enter the dictionary XML file's path: ";
    //cin >> sourcePath;
    sourcePath = "C:/Users/MonPC/Documents/PS2/SB2/ASCII2SHIFTJIS/Variables/items.xml";
    cout << "\nPlease Enter the path to the Input File to convert: ";
    //cin >> inputFile;
    inputFile = "C:/Users/MonPC/Documents/PS2/SB2/ASCII2SHIFTJIS/Variables/Convert.txt";
    size_t found = inputFile.find_last_of("/\\");
    if (found == string::npos)
        outputFile = "Converted.txt";
    else
        outputFile = inputFile.substr(0, found) + "/Converted.txt";//inputFile.substr(found + 1);

    // convert string from ASCII into PCWSTR
    wstring stemp = wstring(sourcePath.begin(), sourcePath.end());
    LPCWSTR sw = stemp.c_str();

    HRESULT hr = CoInitialize(NULL);
    if (SUCCEEDED(hr))
    {
        dictionary* conversionTable = loadDOM(sw);
        ifstream inputStream = ifstream();
        ofstream outputStream = ofstream();
        inputStream.open(inputFile);
        outputStream.open(outputFile);
        
        for (string line; getline(inputStream, line);)
        {
            string convertMe, temp, convertedLine = "";
            BSTR hexLine = SysAllocString(L"");
            int printCount = 0;
            bool addCharacters = false;
            for (string::size_type i = 0; i < line.size(); ++i)
            {
                convertMe = string(1, line[i]);
                if (line[i] == '\\')
                    convertMe = segment(line, &i);

                const char* cstr = convertMe.c_str();
                BSTR getMe = ConvertStringToBSTR(cstr);
                wprintf(getMe);
                hexLine = Concat(hexLine, conversionTable->get(getMe));
                printCount++;

                if (printCount == 2)
                {
                    convertMe = "\n";
                    printCount = 0;
                    addCharacters = true;
                }
                else if (i == line.size() - 1)
                {
                    convertMe = "0000";
                    addCharacters = true;
                }

                if (addCharacters)
                {
                    addCharacters = false;
                    cstr = convertMe.c_str();
                    getMe = ConvertStringToBSTR(cstr);
                    hexLine = Concat(hexLine, getMe);

                    string temp2 = ConvertBSTRToString(hexLine);
                    convertedLine += "hexcode $" + temp2.substr(4, 4) + temp2.substr(0, 4) + temp2.substr(8);
                    SysFreeString(hexLine);
                    hexLine = SysAllocString(L"");
                }
                SysFreeString(getMe);
            }
            cout << "\n" << convertedLine << "\n";
            outputStream << convertedLine << "\n";
            SysFreeString(hexLine);
            temp.clear();
            convertMe.clear();
            convertedLine.clear();
        }

        inputStream.close();
        outputStream.close();
        conversionTable->~dictionary();
        CoUninitialize();
    }
    return 0;
}