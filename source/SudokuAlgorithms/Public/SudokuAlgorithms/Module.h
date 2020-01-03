#pragma once

namespace dd
{

#if !defined(DD_RELEASE)
#define VALIDATE_BIT_BOUNDS
#endif

#ifdef COMPILING_STATIC
#define DD_SUDOKU_ALGORITHMS_API 
#else
#ifdef COMPILING_DLL 
#define DD_SUDOKU_ALGORITHMS_API __declspec(dllexport)
#else
#define DD_SUDOKU_ALGORITHMS_API __declspec(dllimport)
#endif
#endif

#define DD_SudokuAlgorithms_Api DD_SUDOKU_ALGORITHMS_API

	//////////////// CHEAT BIND /////////////
	DD_SUDOKU_ALGORITHMS_API void Bind_SudokuAlgorithms_Module();
	//////////////// CHEAT BIND /////////////
}