#include <assert.h>  
#include <functional>
#include <iostream>

#include <ExampleBoards.h>
#include <Core/Types.h>
#include <SudokuLib/SudokuTypes.h>
#include <SudokuLib/SudokuAlgorithm.h>
#include <SudokuLib/SudokuPrinter.h>

namespace ddahlkvist
{
	bool runTechniques(Board& b, Result& result) {
		SudokuContext context = buildContext(b, result);

		if (context.Unsolved == BitBoard{})
			return true;

		auto techniques = techniques::allTechniques();
		for (auto i = 0ULL, end = techniques.size(); i < end; ++i) {
			techniques::TechniqueFunction& technique = techniques[i];
			if (technique(context))
				return true;
		}
		return false;
	}

	bool solveBoard(Board& b, Result& result)	{
		// do an early iteration to fill candidates and remove the known nodes (from neighbouring solved nodes)
		{
			SudokuContext context = buildContext(b, result);
			techniques::fillUnsolvedWithNonNaiveCandidates(context);
		}

		SolveLedger& ledger = result.ledger;

		u32 iteration = 0;

		bool iterateAgain = true;
		while (iterateAgain && iteration < ledger.MaxEntries) {
#ifdef DD_DEBUG
			b.updateDebugPretty();
#endif
			result.reset();
			if (runTechniques(b, result)) {
				ledger.numNodesChangedInIteration[iteration] = static_cast<u8>(result.size());
				ledger.techniqueUsedInIteration[iteration] = result.Technique;
			}
			iteration++;
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

int main()
{
#ifdef DD_DEBUG
	const bool PrintVerbose = true;
	const bool StopOnFirstUnsolved = false;
#else
	const bool PrintVerbose = false;
	const bool StopOnFirstUnsolved = false;
#endif

	using namespace std;
	using namespace ddahlkvist;

	Board boards[1000];
	const u32 numBoards = FillBoards(boards, GetRawBoards());

	// work on one specfic board
	//{
	//	const int boardIdx = 21;
	//	Board& board = boards[boardIdx];
	//	Result outcome;

	//	bool solved = solveBoard(board, outcome);

	//	if (PrintVerbose)
	//	{
	//		printSudokuBoard(board);
	//		printCandidateOutput(Techniques::NakedPair);

	//		if (solved)
	//			validateSolvedCorectly(board);
	//		else
	//			validateNoDuplicates(board);

	//		cout << "BoardIndex: " << boardIdx << "\t\t" << (solved ? "solved" : "unsolved") << endl;
	//	}
	//}

	// run all
	u32 i = 0;	
	u32 solvedCount = 0;
	for (; i < numBoards; ++i) {
		Board& board = boards[i];
		Result outcome;

		bool solved = solveBoard(board, outcome);
		solvedCount += solved;
		
		if (PrintVerbose)
		{
			printSudokuBoard(board);
			printCandidateOutput(Techniques::NakedPair, outcome.ledger);
		
			if (solved)
				validateSolvedCorectly(board);
			else
				validateNoDuplicates(board); 
			
			cout << "BoardIndex: " << i << "\t\t" << (solved ? "solved" : "unsolved") << endl;
		}

		if constexpr (StopOnFirstUnsolved && !solved)
			break;
	}

	cout << "-------------------" << endl;
	cout << "DONE!!" << endl;
	printf("Execution Finished: Solved=%u/%u\n", solvedCount, i);

	return 0;
}
