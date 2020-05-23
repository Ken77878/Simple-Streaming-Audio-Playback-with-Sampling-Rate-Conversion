
#ifndef CHARACTER_CODE_MACRO
#define CHARACTER_CODE_MACRO

#if (defined WIN32 && defined UNICODE)
#define TSTRING wstring
#define tifstream wifstream
#define tofstream wofstream
#define tprintf wprintf
#define tstringstream wstringstream
#define to_tstring to_wstring
#define tcout wcout
#define tcerr wcerr
#define tstrcmp wcscmp
#define tfprintf fwprintf
#define tstrrchr wcsrchr
#define tfopen_s _wfopen_s
#define _tsopen_s _wsopen_s
#define tstrerror_s   _wcserror_s

#else
#define TSTRING string
#define tifstream ifstream
#define tofstream ofstream
#define tprintf printf
#define tstringstream stringstream
#define to_tstring to_string
#define tcout cout
#define tcerr cerr
#define tstrcmp strcmp
#define tfprintf fprintf
#define tstrrchr strrchr
#define tfopen_s fopen_s
#define _tsopen_s _sopen_s
#define tstrerror_s   strerror_s
#endif

#ifndef WIN32
#define TEXT(quote) quote
#define TCHAR char
#endif

#endif
