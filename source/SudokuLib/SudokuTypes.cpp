#include <SudokuLib/sudokulib_module.h>
#include <SudokuLib/SudokuTypes.h>
#include <BoardUtils.h>

namespace ddahlkvist
{
	SudokuContext buildContext(Board& b, Result& r) 
	{
		SudokuContext ctx{ b, r };

		BoardBits::fillBitsSolved(ctx.SolvedValues, ctx.Solved, b);
		ctx.Unsolved = ctx.Solved.invert();
		BoardBits::buildCandidateBoards(ctx.AllCandidates, ctx.Unsolved, b);
		BoardBits::AllDimensions(ctx.AllDimensions);

		return ctx;
	}

}