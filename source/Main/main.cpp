#include <assert.h>  
#include <functional>
#include <iostream>

#include <ExampleBoards.h>
#include <Validations.h>
#include <Core/Types.h>
#include <SudokuAlgorithms/SudokuTypes.h>
#include <SudokuAlgorithms/SudokuAlgorithm.h>
#include <SudokuPrinter.h>

namespace dd
{
	SudokuContext buildContext(Board& b, Result& r) {
		return SudokuContext{
			b,
			r,
			BoardBits::bitsSolved(b),
			BoardBits::bitsUnsolved(b),
			BoardBits::buildCandidateBoards(b),
			BoardBits::AllDimensions()
		};
	}

	bool runTechniques(Board& b, Result& result) {
		SudokuContext context = buildContext(b, result);

		if (context.Unsolved == BitBoard{})
			return true;

		if (techniques::removeNaiveCandidates(context)) {
			return true;
		}

		if (techniques::removeNakedSingle(context)) {
			return true;
		}

		if (techniques::removeHiddenSingle(context)) {
			return true;
		}

		if (techniques::removeNakedPair(context)) {
			return true;
		}

		if (techniques::removeNakedTriplet(context)) {
			return true;
		}

		return false;
	}

	bool beginSolveBoard(Board& b, Result& result)	{
		// do an early iteration to fill candidates and remove the known nodes (from neighbouring solved nodes)
		{
			SudokuContext context = buildContext(b, result);
			techniques::fillUnsolvedWithNonNaiveCandidates(context);
		}

		g_numIterations = 0;
		bool iterateAgain = true;
		while (iterateAgain && g_numIterations < MaxIterations) {
#ifdef DD_DEBUG
			b.updateDebugPretty();
#endif
			result.reset();
			if (runTechniques(b, result)) {
				g_nodesChangedInIteration[g_numIterations] = static_cast<u8>(result.size());
				g_techniqueUsedInIteration[g_numIterations] = result.Technique;
			}
			g_numIterations++;
			iterateAgain = result.size() != 0;
		}

		u32 postNumSolved = 0;
		for (Node& n : b.Nodes)
		{
			if (n.isSolved())
				postNumSolved++;
		}
		return postNumSolved == BoardSize;
	}
}

void runValidations()
{
	using namespace dd;

	validateCandidates();
	validateBuildBoardFromLayout();
	validateBitHelpers();

	validateCandidateAddAndSimpleRemoval();
	validateTechniques();
}

int main()
{
	const bool PerformanceRun = false;
	const bool Validate = true;
	const bool StopOnFirstUnsolved = false;

	if(Validate)
		runValidations();

	using namespace std;
	using namespace dd;

	Board boards[1000];
	const u32 numBoards = FillBoards(boards, GetRawBoards());
	
	u32 i = 0;
	
	for (; i < numBoards; ++i) {
		Board& board = boards[i];
		Result outcome;

		bool solved;
		{
			solved = beginSolveBoard(board, outcome);
		}
		if (!PerformanceRun)
		{
			printCandidateOutput(Techniques::NakedPair);
			printSudokuBoard(board);
			if (solved)
				validateSolvedCorectly(board);
			else
				validateNoDuplicates(board);
			cout << "BoardIndex: " << i << "\t\t" << (solved ? "solved" : "unsolved") << endl;

			if (StopOnFirstUnsolved && !solved)
				break;
		}
	}

	cout << "-------------------" << endl;
	cout << "DONE!!" << endl;
	cout << "Iterated over: " << i << " boards" << endl;
	cin.get();
	return 0;
}
