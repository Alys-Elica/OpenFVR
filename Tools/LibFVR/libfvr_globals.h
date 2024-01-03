#ifndef LIBFVR_GLOBALS_H
#define LIBFVR_GLOBALS_H

#ifdef _MSC_VER // Microsoft Visual Studio

#ifdef LIBFVR_EXPORTS
#define LIBFVR_EXPORT __declspec(dllexport)
#else
#define LIBFVR_EXPORT __declspec(dllimport)
#endif

#elif defined(__GNUC__) || defined(__clang__) // GCC or Clang

#ifdef LIBFVR_EXPORTS
#define LIBFVR_EXPORT __attribute__((visibility("default")))
#else
#define LIBFVR_EXPORT
#endif

#else

#error "Compiler not supported!"

#endif

#endif // LIBFVR_GLOBALS_H
