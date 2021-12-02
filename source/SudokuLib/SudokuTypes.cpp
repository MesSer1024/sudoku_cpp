#include <SudokuLib/sudokulib_module.h>
#include <SudokuLib/SudokuTypes.h>
#include <BoardUtils.h>

namespace ddahlkvist
{
	SudokuContext buildContext(Board& b, Result& r) 
	{
		SudokuContext ctx{ b, r };

		BoardBits::fillBitsSolved(ctx.SolvedValues, ctx.Solved, b);
		BoardBits::buildCandidateBoards(ctx.AllCandidates, b);
		BoardBits::AllDimensions(ctx.AllDimensions);
		ctx.Unsolved = ctx.Solved.invert();

		return ctx;
	}

}