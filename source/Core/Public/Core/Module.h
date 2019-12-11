#pragma once

namespace mn
{

#ifdef COMPILING_STATIC
#define DD_CORE_API 
#else
#ifdef COMPILING_DLL 
#define DD_CORE_API __declspec(dllexport)
#else
#define DD_CORE_API __declspec(dllimport)
#endif
#endif

#define DD_Core_Api DD_CORE_API

	//////////////// CHEAT BIND /////////////
	DD_CORE_API void Bind_Core_Module();
	//////////////// CHEAT BIND /////////////

}