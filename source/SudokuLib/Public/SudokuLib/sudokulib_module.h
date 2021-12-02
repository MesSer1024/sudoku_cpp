#pragma once

namespace ddahlkvist
{

#define MSVC_COMPILER 1

#if defined(MSVC_COMPILER)
#define IMPORT_DLL __declspec(dllimport)
#define EXPORT_DLL __declspec(dllexport)
#else
#define IMPORT_DLL __attribute__ ((visibility("default")))
#define EXPORT_DLL __attribute__ ((visibility("default")))
#endif

#if defined(BUILD_COMPILE_DLL)
#if defined(BUILD_EXPORT_SUDOKULIB_MODULE)
#define SUDOKULIB_PUBLIC EXPORT_DLL
#else
#define SUDOKULIB_PUBLIC IMPORT_DLL
#endif
#else
#define SUDOKULIB_PUBLIC
#endif

#if defined(BUILD_INTERNAL_ACCESS_SUDOKULIB_MODULE) || defined(BUILD_EXPORT_SUDOKULIB_MODULE)
#define SUDOKULIB_INTERNAL SUDOKULIB_PUBLIC
#else
#define SUDOKULIB_INTERNAL
#endif

}

namespace ddahlkvist::sudokulib_module
{
	SUDOKULIB_PUBLIC void bind(void* data);
}
