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
#if defined(BUILD_EXPORT_CORE_MODULE)
#define CORE_PUBLIC EXPORT_DLL
#else
#define CORE_PUBLIC IMPORT_DLL
#endif
#else
#define CORE_PUBLIC
#endif

//#define CORE_INTERNAL CORE_PUBLIC
#if defined(BUILD_INTERNAL_ACCESS_CORE_MODULE) || defined(BUILD_EXPORT_CORE_MODULE)
#define CORE_INTERNAL CORE_PUBLIC
#else
#define CORE_INTERNAL
#endif

}

namespace ddahlkvist::core_module
{
	CORE_PUBLIC void bind(void* data);
}
