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
	bool runTechniques(Board& b, Result& result) {
		SudokuContext context{ b, result,
			BoardBits::bitsSolved(b),
			BoardBits::bitsUnsolved(b),
			buildCandidateBoards(b),
			BoardBits::AllDimensions()
		};

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

		return false;
	}

	bool beginSolveBoard(Board& b, Result& result)	{
		const u32 Max = 1000;
		static u8 nodesChangedInIteration[Max];
		static u8 techniqueUsedInIteration[Max];

		// do an early iteration to fill candidates and remove the known nodes (from neighbouring solved nodes)
		{
			SudokuContext context{ b, result,
				BoardBits::bitsSolved(b),
				BoardBits::bitsUnsolved(b),
				buildCandidateBoards(b),
				BoardBits::AllDimensions()
			};

			techniques::fillAllUnsolvedWithAllCandidates(context);
			techniques::removeNaiveCandidates(context);
		}

		int i = 0;
		bool iterateAgain = true;
		while (iterateAgain && i < Max) {
			result.reset();
			if (runTechniques(b, result)) {
				nodesChangedInIteration[i] = static_cast<u8>(result.size());
				techniqueUsedInIteration[i] = static_cast<u8>(result.Technique);
			}
			i++;
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
	validateUtilities();
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

	u32 currBoardIndex = 0;
	auto exampleBoards = GetBoards();
	for (SudokuBoardRaw& boardRaw : exampleBoards) {
		Board board = Board::fromString(boardRaw.c_str());
		Result outcome;

		bool solved;
		{
			solved = beginSolveBoard(board, outcome);
		}
		if (!PerformanceRun)
		{
			printSudokuBoard(board);
			if (solved)
				validateSolvedCorectly(board);
			else
				validateNoDuplicates(board);
			cout << "BoardIndex: " << currBoardIndex << "\t\t" << (solved ? "solved" : "unsolved") << endl;

			if (StopOnFirstUnsolved && !solved)
				break;
		}
		currBoardIndex++;
	}

	cout << "DONE!" << endl;
	cin.get();
	return 0;
}
