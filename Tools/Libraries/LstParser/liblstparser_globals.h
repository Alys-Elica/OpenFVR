#ifndef LIBLSTPARSER_GLOBALS_H
#define LIBLSTPARSER_GLOBALS_H

#ifdef _MSC_VER // Microsoft Visual Studio

#ifdef LIBLSTPARSER_EXPORTS
#define LIBLSTPARSER_EXPORT __declspec(dllexport)
#else
#define LIBLSTPARSER_EXPORT __declspec(dllimport)
#endif

#elif defined(__GNUC__) || defined(__clang__) // GCC or Clang

#ifdef LIBLSTPARSER_EXPORTS
#define LIBLSTPARSER_EXPORT __attribute__((visibility("default")))
#else
#define LIBLSTPARSER_EXPORT
#endif

#else

#error "Compiler not supported!"

#endif

#endif // LIBLSTPARSER_GLOBALS_H
