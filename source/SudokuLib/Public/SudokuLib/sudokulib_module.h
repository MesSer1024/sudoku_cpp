#pragma once

namespace dd
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
#if defined(BUILD_EXPORT_TEMPLATE_MODULE)
#define TEMPLATE_PUBLIC EXPORT_DLL
#else
#define TEMPLATE_PUBLIC IMPORT_DLL
#endif
#else
#define TEMPLATE_PUBLIC
#endif

//#define TEMPLATE_INTERNAL TEMPLATE_PUBLIC
#if defined(BUILD_INTERNAL_ACCESS_TEMPLATE_MODULE) || defined(BUILD_EXPORT_TEMPLATE_MODULE)
#define TEMPLATE_INTERNAL TEMPLATE_PUBLIC
#else
#define TEMPLATE_INTERNAL
#endif

namespace sudokulib
{
void bind(void* data);
unsigned int getProjectId();
}

}