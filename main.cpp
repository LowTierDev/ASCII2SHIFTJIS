#include <iostream>
#include "computation.h"
#include "loadXML.h"
#include <fstream>
#include <comutil.h>
#include <iomanip>
#include <sstream>
#include <algorithm>

using std::cin;
using std::cout;
using std::hex;
using std::stoi;
using std::size_t;
using std::string;
using std::ifstream;
using std::ofstream;
using std::wstring;
using _com_util::ConvertStringToBSTR;
using _com_util::ConvertBSTRToString;

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

template< typename T >
string getPS2Address(T i)
{
    std::stringstream stream;
    stream //<< "0x"
        << std::setfill('0') << std::setw(8) // sizeof(your_type)*2
        << std::hex << i;
    string returnMe = stream.str();
    std::transform(returnMe.begin(), returnMe.end(), returnMe.begin(), ::toupper);
    return returnMe;//stream.str();
}

string* getPaths()
{
    string* returnMe = new string[3];
    cout << "Please Enter the dictionary XML file's path: ";
    //cin >> sourcePath;
    returnMe[0] = "C:/Users/MonPC/Documents/PS2/SB2/ASCII2SHIFTJIS/Variables/items.xml";
    cout << "\nPlease Enter the path to the Input File to convert: ";
    //cin >> inputFile;
    returnMe[1] = "C:/Users/MonPC/Documents/PS2/SB2/ASCII2SHIFTJIS/Variables/Convert.txt";
    size_t found = returnMe[1].find_last_of("/\\");
    if (found == string::npos)
        returnMe[2] = "Converted.txt";
    else
        returnMe[2] = returnMe[1].substr(0, found) + "/Converted.txt";//inputFile.substr(found + 1);

    return returnMe;
}


int main()
{
    // The File Paths
    string* thePaths = getPaths();
    string sourcePath = thePaths[0];
    string inputFile = thePaths[1];
    string outputFile = thePaths[2];

    // 
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
        bool initialLine = true;
        int cAddress, nAddress;
        
        for (string line; getline(inputStream, line);)
        {
            if (initialLine)
            {
                size_t pos = line.find("$");
                cAddress = stoi(line.substr(pos + 1), nullptr, 16);
                nAddress = cAddress;
                initialLine = false;
                continue;
            }

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
                BSTR definition = conversionTable->get(getMe);
                if (definition == NULL)
                {
                    cout << "Couldn't find: ";
                    wprintf(getMe);
                    break;
                }
                else
                    hexLine = Concat(hexLine, definition);
                printCount++;
                nAddress += 0x2;

                if (printCount == 2)
                {
                    convertMe = "\n";
                    printCount = 0;
                    addCharacters = true;
                }
                else if (i == line.size() - 1)
                {
                    convertMe = "0000\n";
                    addCharacters = true;
                    nAddress += 0x2;
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
            cout << "\n" << "address $" << getPS2Address(cAddress) << "\n" << convertedLine << "\n";
            outputStream << "address $" << getPS2Address(cAddress) << "\n" << convertedLine << "\n";
            cAddress = nAddress;
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

    delete[] thePaths;
    return 0;
}