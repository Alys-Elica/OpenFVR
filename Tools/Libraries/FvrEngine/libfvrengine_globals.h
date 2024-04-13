#ifndef LIBFVRENGINE_GLOBALS_H
#define LIBFVRENGINE_GLOBALS_H

#ifdef _MSC_VER // Microsoft Visual Studio

#ifdef LIBFVRENGINE_EXPORTS
#define LIBFVRENGINE_EXPORT __declspec(dllexport)
#else
#define LIBFVRENGINE_EXPORT __declspec(dllimport)
#endif

#elif defined(__GNUC__) || defined(__clang__) // GCC or Clang

#ifdef LIBFVRENGINE_EXPORTS
#define LIBFVRENGINE_EXPORT __attribute__((visibility("default")))
#else
#define LIBFVRENGINE_EXPORT
#endif

#else

#error "Compiler not supported!"

#endif

#endif // LIBFVRENGINE_GLOBALS_H
