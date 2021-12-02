#pragma once

#include <iostream>
#include <Core/Types.h>
#include <SudokuLib/SudokuTypes.h>
#include <SudokuLib/TechniqueMeta.h>

namespace ddahlkvist 
{

	constexpr char const* LineSymbol{ "-----------------\n" };
	constexpr char const* EmptyNode{ "." };

	SUDOKULIB_PUBLIC void validateNoDuplicates(const Board& b);

	SUDOKULIB_PUBLIC void validateSolvedCorectly(const Board& b);

	SUDOKULIB_PUBLIC void printCandidateOutput(Techniques lowestPrinted, const SolveLedger& ledger);

	SUDOKULIB_PUBLIC void printSudokuBoard(Board& b);

	SUDOKULIB_PUBLIC void printCandidates(Board& b, const BitBoard& checkMask);

	SUDOKULIB_PUBLIC const char* getTechniqueName(Techniques technique);
}