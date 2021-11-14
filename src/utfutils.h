#ifndef _UTFUTILS_H
#define _UTFUTILS_H
#include <string>

typedef std::string string;
typedef std::wstring wstring;
#define S(x) string(x)

size_t utf8len(const char* s);
size_t utf8clen(const char* s);
size_t utf8pos(const char* s, size_t inpos);
size_t utf8cpos(const char* s, size_t inpos);
size_t utf8findcpos(const char* s, float inpos);
char utf8csize(const unsigned char* c);

wstring utf8To16(const char* in);
string utf16To8(const wchar_t* in);

#endif
