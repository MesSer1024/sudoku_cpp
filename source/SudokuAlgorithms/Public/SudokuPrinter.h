#pragma once

#include <iostream>
#include <Core/Types.h>
#include <SudokuAlgorithms/SudokuTypes.h>

namespace dd 
{
	constexpr u32 MaxIterations = 1000;
	static u8 g_nodesChangedInIteration[MaxIterations];
	static Techniques g_techniqueUsedInIteration[MaxIterations];
	static u32 g_numIterations;

	constexpr char const* LineSymbol{ "-----------------\n" };
	constexpr char const* EmptyNode{ "." };

	void validateNoDuplicates(const Board& b) {
		auto allDimensions = BoardBits::AllDimensions();
		for (auto dimension : allDimensions) {
			const BitBoard solvedNodes = (BoardBits::bitsSolved(b) & dimension);
			const u32 solvedNodeCount = solvedNodes.countSetBits();
			const u32 solvedValues = countCandidates(BoardUtils::buildValueMaskFromSolvedNodes(b.Nodes, solvedNodes));
			if(solvedValues != solvedNodeCount)
				assert(false);
		}
	}

	void validateSolvedCorectly(const Board& b) {
		assert(BoardBits::bitsUnsolved(b).countSetBits() == 0);
		auto rows = BoardBits::AllRows();
		auto cols = BoardBits::AllColumns();
		auto blocks = BoardBits::AllBlocks();

		for (auto dimension : rows) {
			const u32 solvedValues = BoardUtils::buildValueMaskFromSolvedNodes(b.Nodes, dimension);
			assert(solvedValues == Candidates::All);
		}
		
		for (auto dimension : cols) {
			const u32 solvedValues = BoardUtils::buildValueMaskFromSolvedNodes(b.Nodes, dimension);
			assert(solvedValues == Candidates::All);
		}
		
		for (auto dimension : blocks) {
			const u32 solvedValues = BoardUtils::buildValueMaskFromSolvedNodes(b.Nodes, dimension);
			assert(solvedValues == Candidates::All);
		}
	}

	void printCandidateOutput(Techniques lowestPrinted) {


		for (uint i = 0, end = g_numIterations; i < end; ++i) {
			if (g_techniqueUsedInIteration[i] >= lowestPrinted)
			{
				const char* techniqueName = TechniqueNameLookup[g_techniqueUsedInIteration[i]];
				printf("%u: Made %u changes with technique %s\n", i, g_nodesChangedInIteration[i], techniqueName);
			}
		}
	}

	void printSudokuBoard(Board& b) {
		using namespace std;
		cout << endl << LineSymbol;
		uint i = 0;
		for (const Node& n : b.Nodes) {
			if (i % 9 == 0 && i > 0)
				cout << endl;
			if (i % 27 == 0 && i > 0)
				cout << endl;
			if (i % 3 == 0)
				cout << "  ";
			if (n.isSolved())
				cout << n.getValue();
			else
				cout << EmptyNode;
			i++;
		}
		cout << endl << LineSymbol;
	}

	void printCandidates(Board& b, const BitBoard& checkMask) {
		using namespace std;
		cout << endl << LineSymbol << endl;
		uint i = 0;

		for (const Node& n : b.Nodes) {
			const bool validate = checkMask.test(i);
			if (validate && !n.isSolved()) {
				u8 candidates[9];
				u32 numCandidates = n.fillCandidateIds(candidates);

				printf("\nidx=%u candidates:%u = {", i, numCandidates);
				for (uint i = 0; i < numCandidates; ++i)
					printf("%s%u", (i>0 ? ",":""), candidates[i]);
				printf("}");
			}
			i++;
		}
	}
}