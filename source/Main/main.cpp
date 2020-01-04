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
		if (techniques::removeNaiveCandidates(b, result)) {
			result.Technique = TechniqueUsed::NaiveCandidates;
			return true;
		}

		if (techniques::removeNakedSingle(b, result)) {
			result.Technique = TechniqueUsed::NakedSingle;
			return true;
		}

		if (techniques::removeHiddenSingle(b, result)) {
			result.Technique = TechniqueUsed::HiddenSingle;
			return true;
		}

		return false;
	}

	bool beginSolveBoard(Board& b, Result& result)	{
		u32 preNumSolved = 0;
		for (Node& n : b.Nodes)
		{
			if (n.isSolved())
				preNumSolved++;
		}

		techniques::fillAllUnsolvedWithAllCandidates(b);
		techniques::removeNaiveCandidates(b, result);

		int i = 0;
		bool unsolved = true;
		while (unsolved && i < 2000) {
			result.reset();
			if (runTechniques(b, result)) {
				if(result.Technique > TechniqueUsed::NaiveCandidates)
					printf("Made %u changes with technique %u\n", result.size(), result.Technique);
			}
			i++;
			unsolved = BoardBits::bitsUnsolved(b).count() != 0;
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
	const bool Validate = true;
	const bool StopOnFirstUnsolved = true;

	if(Validate)
		runValidations();

	using namespace std;
	using namespace dd;

	u32 currBoardIndex = 0;
	auto exampleBoards = GetBoards();
	for (SudokuBoardRaw& boardRaw : exampleBoards) {
		Board board = Board::fromString(boardRaw.c_str());
		Result outcome;

		bool solved = beginSolveBoard(board, outcome);
		printSudokuBoard(board);
		if (solved)
			validateSolvedCorectly(board);
		else
			validateNoDuplicates(board);
		cout << "BoardIndex: " << currBoardIndex << "\t\t" << (solved ? "solved" : "unsolved") << endl;

		if (StopOnFirstUnsolved && !solved)
			break;
		currBoardIndex++;
	}

	cin.get();
	return 0;
}
