/********************************************************************
 *
 *      LIL Is a Language
 *
 *      AUTHORS: Miro Keller
 *
 *      COPYRIGHT: ©2020-today:  All Rights Reserved
 *
 *      LICENSE: see LICENSE file
 *
 *      This file implements a wrapper for basic strings
 *
 ********************************************************************/


#include "LILShared.h"
#include "LILString.h"

using namespace LIL;

namespace LIL
{
    class LILStringPrivate
    {
        friend class LILString;

        LILStringPrivate()
        {

        }
        std::string string;
    };
}

LILString LILString::number(LILUnitI8 num)
{
    std::ostringstream out;
    out << std::dec << num;
    
    return LILString(out.str());
}

LILString LILString::number(LILUnitI16 num)
{
    std::ostringstream out;
    out << std::dec << num;
    
    return LILString(out.str());
}

LILString LILString::number(LILUnitI32 num)
{
    std::ostringstream out;
    out << std::dec << num;
    
    return LILString(out.str());
}

LILString LILString::number(LILUnitI64 num)
{
    std::ostringstream out;
    out << std::dec << num;
    
    return LILString(out.str());
}

LILString LILString::number(LILUnitF32 num)
{
    
    std::ostringstream out;
    out.precision(LILString::_count_decimals(num));
    out << std::fixed << num;
    
    return LILString(out.str());
}

LILString LILString::number(LILUnitF64 num)
{

    std::ostringstream out;
    out.precision(LILString::_count_decimals(num));
    out << std::fixed << num;

    return LILString(out.str());
}

unsigned int LILString::_count_decimals(LILUnitF64 num)
{
    unsigned int count = 0;
    num = std::abs((long long)num);
    num = num - int(num);
    while (std::abs((long long)num) >= 0.0000001)
    {
        num = num * 10;
        if (std::isinf(num))
            return 0;
        count += 1;
        num = num - int(num);
    }
    return count;
}

LILString LILString::format(const LILString input, ...)
{
    std::string fmt = input.data();
    int size = ((int)fmt.size()) * 2 + 50;   // Use a rubric appropriate for your code
    std::string str;
    va_list ap;
    while (1) {     // Maximum two passes on a POSIX system...
        str.resize(size);
        va_start(ap, input);
        int n = vsnprintf((char *)str.data(), size, fmt.c_str(), ap);
        va_end(ap);
        if (n > -1 && n < size) {  // Everything worked
            str.resize(n);
            return LILString(str);
        }
        if (n > -1)  // Needed size returned
            size = n + 1;   // For null char
        else
            size *= 2;      // Guess at a larger size (OS specific)
    }
    return LILString(str);
}

LILString LILString::join(std::vector<LILString> strings, const char * glue)
{
    std::string ret;
    for (size_t i = 0, j = strings.size(); i<j; ++i)
    {
        const LILString & string = strings[i];
        ret += string.data();
        if (i < j-1)
        {
            ret += glue;
        }
    }
    return LILString(ret);
}

LILString::LILString()
: d(new LILStringPrivate)
{

}

LILString::LILString(const LILString & other)
: d(new LILStringPrivate)
{
    d->string = other.d->string;
}

LILString::~LILString()
{
    delete d;
}

LILString::LILString(const char * cStr)
: d(new LILStringPrivate)
{
    d->string = cStr;
}

LILString::LILString(std::string str)
: d(new LILStringPrivate)
{
    d->string = str;
}

LILString::LILString(LILChar c)
: d(new LILStringPrivate)
{
    d->string = c.data();
}

const std::string & LILString::data() const
{
    return d->string;
}

const char * LILString::chardata() const
{
    return (char *)d->string.data();
}

std::string::iterator LILString::begin() const
{
    return d->string.begin();
}

std::string::iterator LILString::end() const
{
    return d->string.end();
}

LILString & LILString::operator=(const LILString &other)
{
    d->string = other.data();
    return *this;
}

void LILString::clear()
{
    d->string.clear();
}

LILString & LILString::append(const LILString &other)
{
    d->string = d->string + other.data();
    return *this;
}

LILString & LILString::append(const LILChar &c)
{
    const LILUnitI64 intval = c.data();
    d->string += static_cast<const char>(intval);
    return *this;
}

LILString & LILString::replace(std::string & other)
{
    d->string.clear();
    d->string.append(other);
    return *this;
}

void LILString::truncate(size_t pos)
{
    d->string.erase(d->string.begin()+pos, d->string.end());
}

const bool LILString::operator==(const LILString &other) const
{
    return d->string == other.data();
}

const bool LILString::operator==(const char * other) const
{
    return d->string == other;
}

float LILString::toFloat() const
{
    LILString val = this->stripQuotes();
    try
    {
        float ret = std::stof(val.data());
        return ret;
    }
    catch (const std::invalid_argument & ia)
    {
        
    }
    return 0.;
}

double LILString::toDouble() const
{
    LILString val = this->stripQuotes();
    try
    {
        double ret = std::stod(val.data());
        return ret;
    }
    catch (const std::invalid_argument & ia)
    {

    }
    return 0.;
}

short int LILString::toShortInt() const
{
    LILString val = this->stripQuotes();
    try
    {
        short int ret = std::stoi(val.data());
        return ret;
    }
    catch (const std::invalid_argument & ia)
    {
        
    }
    return 0;
}

char LILString::toChar() const
{
    LILString val = this->stripQuotes();
    try
    {
        char ret = std::stoi(val.data());
        return ret;
    }
    catch (const std::invalid_argument & ia)
    {
        
    }
    return 0;
}

int LILString::toInt() const
{
    LILString val = this->stripQuotes();
    try
    {
        int ret = std::stoi(val.data());
        return ret;
    }
    catch (const std::invalid_argument & ia)
    {
        
    }
    return 0;
}

long LILString::toLong() const
{
    LILString val = this->stripQuotes();
    try
    {
        long ret = std::stol(val.data());
        return ret;
    }
    catch (const std::invalid_argument & ia)
    {
        
    }
    return 0;
}

long long LILString::toLongLong() const
{
    LILString val = this->stripQuotes();
    try
    {
        long long ret = std::stoll(val.data());
        return ret;
    }
    catch (const std::invalid_argument & ia)
    {
        
    }
    return 0;
}

unsigned int LILString::toHex() const
{
    return std::stoi(this->stripQuotes().data(), nullptr, 16);
}

LILString LILString::trimmed() const
{
    LILString ret;
    std::string val = d->string;
    if (val.length() > 0)
    {
        std::string wspc (" \t\f\v\n\r");// These are the whitespaces
        //finding the last valid character
        std::string::size_type posafter = val.find_last_not_of(wspc);
        //finding the first valid character
        std::string::size_type posbefore=val.find_first_not_of(wspc);

        if((-1 < (int)posafter) && (-1 < (int)posbefore)) //Just Wsp
        {
            ret = val.substr(posbefore,((posafter+1)-posbefore));
        }
    }
    return ret;
}

LILString LILString::stripQuotes() const
{
    std::string string = d->string;
    size_t position = 0;
    size_t n = 0;
    size_t len = string.length();
    if (string.substr(0, 1) == "\"")
    {
        position = 1;
        if (string.substr(len-1, 1) == "\"")
        {
            n = len - 1 - position;
        }
        else
        {
            n = len - position;
        }
    }
    else if (string.substr(0, 1) == "'")
    {
        position = 1;
        if (string.substr(len-1, 1) == "'")
        {
            n = len - 1 - position;
        }
        else
        {
            n = len - position;
        }
    }
    else if (string.substr(0, 1) == "`")
    {
        position = 1;
        if (string.substr(len-1, 1) == "`")
        {
            n = len - 1 - position;
        }
        else
        {
            n = len - position;
        }
    }
    else
    {
        return LILString(string);
    }
    return LILString(string.substr(position, n));
}

LILString LILString::strip(const LILChar & c) const
{
    std::string str = d->string;
    size_t startIndex = 0;
    for (auto it = str.begin(); it != str.end(); ++it) {
        if (c != *it) {
            break;
        }
        ++startIndex;
    }
    size_t len = str.length();
    size_t endIndex = 0;
    for (auto it = str.rbegin(); it != str.rend(); ++it) {
        if (c != *it) {
            break;
        }
        ++endIndex;
    }
    return LILString(str.substr(startIndex, (len - startIndex - endIndex)));
}

LILString LILString::replaceEscapes() const
{
    std::string string = d->string;
    const char * strdata = string.c_str();
    size_t lastPosition = 0;
    size_t position = 0;
    size_t n;
    size_t len = string.length();
    bool isEscape = false;
    std::string ret;
    for (n=0; n<len; ++n) {
        if (isEscape)
        {
            isEscape = false;
            ret += string.substr(lastPosition, position - lastPosition);
            lastPosition = position + 2;
            
            switch (strdata[n]) {
                case 'n':
                {
                    ret += "\n";
                    break;
                }

                case 't':
                {
                    ret += "\t";
                    break;
                }
                default:
                    break;
            }
        }
        else
        {
            if (strdata[n] == '\\') {
                isEscape = true;
                position = n;
            }
        }
    }
    if (len > lastPosition) {
        ret += string.substr(lastPosition, len - lastPosition);
    }
    return ret;
}

LILString LILString::toUpperFirstCase() const
{
    std::string newString = this->data();
    newString[0] = toupper(newString[0]);
    return newString;
}

bool LILString::isEmpty() const
{
    return d->string.empty();
}

size_t LILString::length() const
{
    return d->string.length();
}

LILString LILString::substr(size_t start, size_t length) const
{
    return LILString(d->string.substr(start, length));
}

LILString LILString::substr(size_t start) const
{
    return LILString(d->string.substr(start));
}

LILString LILString::at(size_t pos) const
{
    return this->substr(pos, 1);
}

bool LILString::contains(const char * needle) const
{
    size_t pos = d->string.find(needle);
    return pos != std::string::npos;
}
