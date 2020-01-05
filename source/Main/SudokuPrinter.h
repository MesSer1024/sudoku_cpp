#pragma once

#include <iostream>
#include <Core/Types.h>
#include <SudokuAlgorithms/SudokuTypes.h>

namespace dd 
{
	constexpr char* LineSymbol{ "-----------------\n" };
	constexpr char* EmptyNode{ "." };

	void validateNoDuplicates(const Board& b) {
		auto allDimensions = BoardBits::AllDimensions();
		for (auto dimension : allDimensions) {
			const BitBoard solvedNodes = (BoardBits::bitsSolved(b) & dimension);
			const u32 solvedNodeCount = solvedNodes.count();
			const u32 solvedValues = countCandidates(buildValueMaskFromSolvedNodes(b.Nodes, solvedNodes));
			if(solvedValues != solvedNodeCount)
				assert(false);
		}
	}

	void validateSolvedCorectly(const Board& b) {
		assert(BoardBits::bitsUnsolved(b).count() == 0);
		auto rows = BoardBits::AllRows();
		auto cols = BoardBits::AllColumns();
		auto blocks = BoardBits::AllBlocks();

		for (auto dimension : rows) {
			const u32 solvedValues = buildValueMaskFromSolvedNodes(b.Nodes, dimension);
			assert(solvedValues == Candidates::All);
		}
		
		for (auto dimension : cols) {
			const u32 solvedValues = buildValueMaskFromSolvedNodes(b.Nodes, dimension);
			assert(solvedValues == Candidates::All);
		}
		
		for (auto dimension : blocks) {
			const u32 solvedValues = buildValueMaskFromSolvedNodes(b.Nodes, dimension);
			assert(solvedValues == Candidates::All);
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

	void printCandidates(Board& b) {
		using namespace std;
		cout << endl << LineSymbol << endl;
		uint i = 0;
		for (const Node& n : b.Nodes) {
			if (!n.isSolved()) {
			}
		}
	}
}