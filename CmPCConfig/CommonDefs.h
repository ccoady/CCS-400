#ifndef COMMON_DEFS_H
#define COMMON_DEFS_H

#include <string>
#include <set>

#ifdef WIN32
   #include <windows.h>
#else
   typedef char* LPTSTR;
   typedef const char* LPCTSTR;
   typedef int   BOOL;
   typedef unsigned int UINT;
   typedef unsigned int* PUINT;
   typedef char TCHAR;
   typedef const char* LPCSTR;
   typedef unsigned long DWORD;
   typedef unsigned short USHORT;
#endif

//#define WSAGetLastError() errno

#ifndef VERIFY
#define VERIFY
#endif

#ifndef ASSERT
#define ASSERT assert
#endif

// definements for unicode support
#ifdef _UNICODE
   typedef std::wstring tstring;
   typedef std::wostream tostream;
   typedef std::wifstream tifstream;
   typedef std::wofstream tofstream;
   typedef std::wostringstream tostringstream;
   typedef std::wistringstream tistringstream;
   typedef std::wstringstream tstringstream;
   typedef std::wstreambuf tstreambuf;

   #define tcin  wcin
   #define tcout wcout
   #define tcerr wcerr
   #define tclog wclog
   #define tcstoul wcstoul
   #define tcschr wcschr
   #define tcsncmp wcsncmp
   #define tisdigit iswdigit
   #define tsprintf swprintf
   #define tcsncpy wcsncpy
   #define ttol wcstol
   #define _sntprintf _snwprintf

   #ifndef _T
   #define _T(x) L ## x
   #endif
#else
   typedef std::string tstring;
   typedef std::ostream tostream;
   typedef std::ifstream tifstream;
   typedef std::ofstream tofstream;
   typedef std::ostringstream tostringstream;
   typedef std::istringstream tistringstream;
   typedef std::stringstream tstringstream;
   typedef std::streambuf tstreambuf;

   #define tcin  cin
   #define tcout cout
   #define tcerr cerr
   #define tclog clog
   #define tcstoul strtoul
   #define tcschr strchr
   #define tcsncmp strncmp
   #define tisdigit isdigit
   #define tsprintf sprintf
   #define tcsncpy strncpy
   #define ttol strtol
   #define _sntprintf _snprintf

   #ifndef _T
   #define _T
   #endif
#endif


#endif //COMMON_DEFS_H
