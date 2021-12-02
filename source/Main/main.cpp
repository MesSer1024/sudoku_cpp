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

		auto&& techniques = techniques::allTechniques();
		for (auto i = 0ULL, end = techniques.size(); i < end; ++i) {
			techniques::TechniqueFunction& technique = techniques[i];
			if (technique(context))
				return true;
		}
		return false;
	}

	bool solveBoard(Board& b, Result& r)	{
		// do an early iteration to fill candidates and remove the known nodes (from neighbouring solved nodes)
		{
			SudokuContext context = buildContext(b, r);
			techniques::fillUnsolvedWithNonNaiveCandidates(context);
		}

		// let each board be solved 100 times to make sure we have more performance data
#ifdef DD_FINAL
		const u32 IterationCount = 100;
		const u32 LastIteration = IterationCount - 1;
#else
		const u32 IterationCount = 1;
		const u32 LastIteration = IterationCount - 1;
#endif

		bool isSolved = false;
		for (uint i = 0; i < IterationCount; ++i)
		{
			Board localBoard = b;
			Result localResult = r;
			SolveLedger& ledger = localResult.ledger;

			u32 iteration = 0;

			bool iterateAgain = true;
			while (iterateAgain && iteration < ledger.MaxEntries) {
#ifdef DD_DEBUG
				localBoard.updateDebugPretty();
#endif
				localResult.reset();
				if (runTechniques(localBoard, localResult)) {
					ledger.numNodesChangedInIteration[iteration] = static_cast<u8>(localResult.size());
					ledger.techniqueUsedInIteration[iteration] = localResult.Technique;
				}
				iteration++;
				iterateAgain = localResult.size() != 0;
			}

			u32 postNumSolved = 0;
			for (Node& n : localBoard.Nodes)
			{
				if (n.isSolved())
					postNumSolved++;
			}

			if (i == LastIteration)
			{
				isSolved = postNumSolved == BoardSize;
				b = localBoard;
				r = localResult;			
			}
		}

		return isSolved;
	}
}

int main()
{
#ifdef DD_DEBUG
	constexpr bool PrintVerbose = true;
	constexpr bool StopOnFirstUnsolved = false;
#else
	constexpr bool PrintVerbose = false;
	constexpr bool StopOnFirstUnsolved = false;
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
		
		if constexpr (PrintVerbose)
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
