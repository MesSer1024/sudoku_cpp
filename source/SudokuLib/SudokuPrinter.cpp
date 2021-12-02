#pragma once

#include <iostream>
#include <Core/Types.h>
#include <SudokuLib/SudokuTypes.h>
#include <SudokuLib/TechniqueMeta.h>
#include <SudokuLib/SudokuPrinter.h>
#include <BoardUtils.h>

namespace ddahlkvist 
{
	//u8 g_nodesChangedInIteration[MaxIterations];
	//Techniques g_techniqueUsedInIteration[MaxIterations];
	//u32 g_numIterations;

	const char* getTechniqueName(Techniques technique)
	{

		using TechniqueName = std::pair<Techniques, const char*>;
		std::map<Techniques, const char*> g_TechniqueNameLookup = {
			TechniqueName(Techniques::None, "NA"),
			TechniqueName(Techniques::NaiveCandidates, "Naive"),
			TechniqueName(Techniques::NakedSingle, "Naked Single"),
			TechniqueName(Techniques::HiddenSingle, "Hidden Single"),
			TechniqueName(Techniques::NakedPair, "Naked Pair"),
			TechniqueName(Techniques::NakedTriplet, "Naked Triplet"),
			TechniqueName(Techniques::HiddenPair, "Hidden Pair"),
			TechniqueName(Techniques::HiddenTriplet, "Hidden Triplet"),
			TechniqueName(Techniques::NakedQuad, "Naked Quad"),
			TechniqueName(Techniques::NakedQuad, "Hidden Quad"),
			TechniqueName(Techniques::PointingPair, "PointingPair"),
			TechniqueName(Techniques::BoxLineReduction, "BoxLineReduction"),
			TechniqueName(Techniques::X_Wing, "X-Wing"),
			TechniqueName(Techniques::Y_Wing, "Y-Wing"),
			TechniqueName(Techniques::SingleChain, "Single Chain"),
			TechniqueName(Techniques::UniqueRectangle, "Unique Rectangle"),
		};
		return g_TechniqueNameLookup[technique];
	}



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

	void printCandidateOutput(Techniques lowestPrinted, const SolveLedger& ledger) {
		for (uint i = 0, end = ledger.numIterations; i < end; ++i) {
			const auto& usedTechnique = ledger.techniqueUsedInIteration[i];
			if (usedTechnique >= lowestPrinted)
			{
				const char* techniqueName = getTechniqueName(usedTechnique);
				printf("%u: Made %u changes with technique %s\n", i, ledger.numNodesChangedInIteration[i], techniqueName);
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
				for (uint j = 0; j < numCandidates; ++j)
					printf("%s%u", (j>0 ? ",":""), candidates[j]);
				printf("}");
			}
			i++;
		}
	}
}