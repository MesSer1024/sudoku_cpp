#include <SudokuLib/sudokulib_module.h>
#include <SudokuLib/SudokuTypes.h>
#include <BoardUtils.h>

namespace ddahlkvist
{
	SudokuContext buildContext(Board& b, Result& r) 
	{
		return SudokuContext{
			b,
			r,
			BoardBits::bitsSolved(b),
			BoardBits::bitsUnsolved(b),
			BoardBits::buildCandidateBoards(b),
			BoardBits::AllDimensions()
		};
	}

}