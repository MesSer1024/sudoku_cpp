#pragma once

#include <SudokuLib/sudokulib_module.h>
#include <SudokuLib/SudokuTypes.h>

namespace ddahlkvist::techniques
{
	//---------------- Sudoku Solver -------------
	//http://www.thonky.com/sudoku/

	//Check for solved cells
	//Show Possibles	No

	//		x: Basic Candidates				removeNaiveCandidates() [All solved neighbours toMask -> remove as candidates on unsolved]
	//		0: Naked Single					removeNakedSingle()		[ All unsolved nodes -> If a node only has one candidate, it can be solved]
	//		1: Hidden Singles				removeHiddenSingle()	[ All unsolved neighbours -> if this is only occurance of candidate, it can be solved]
	//		2: Naked Pairs/Triples			removeNakedPair()		[ All unsolved in dimension -> if 2 nodes only have 2 candidates and share them, all other shared neighbours can remove those candidates]
	//										removeNakedTriplet()	[ All unsolved in dimension -> if 3 nodes only have 3 candidates and share them, all other shared neighbours can remove those candidates]
	//		3: Hidden Pairs/Triples	 
	//		4: Naked Quads	 
	//		5: Pointing Pairs	 
	//		6: Box/Line Reduction

	//Tough Strategies	
	//		7: X-Wing	 
	//		8: Simple Colouring	 
	//		9: Y-Wing	 
	//		10: Sword-Fish	 
	//		11: XYZ Wing	 
	//Diabolical Strategies	
	//		12: X-Cycles	 
	//		13: XY-Chain	 
	//		14: 3D Medusa	 
	//		15: Jelly-Fish	 
	//		16: Unique Rectangles	 
	//		17: Extended Unique Rect.	 
	//		18: Hidden Unique Rect's	 
	//		19: WXYZ Wing	 
	//		20: Aligned Pair Exclusion	 
	//Extreme Strategies	
	//		21: Grouped X-Cycles	 
	//		22: Empty Rectangles	 
	//		23: Finned X-Wing	 
	//		24: Finned Sword-Fish	 
	//		25: Altern. Inference Chains	 
	//		26: Sue-de-Coq	 
	//		27: Digit Forcing Chains	 
	//		28: Nishio Forcing Chains	 
	//		29: Cell Forcing Chains	 
	//		30: Unit Forcing Chains	 
	//		31: Almost Locked Sets	 
	//		32: Death Blossom	 
	//		33: Pattern Overlay Method	 
	//		34: Quad Forcing Chains	 
	//"Trial and Error"	
	//		35: Bowman's Bingo

	SUDOKULIB_PUBLIC void fillUnsolvedWithNonNaiveCandidates(SudokuContext& p);

	SUDOKULIB_PUBLIC bool removeNaiveCandidates(SudokuContext& p);
	SUDOKULIB_PUBLIC bool removeNakedSingle(SudokuContext& p);

	// [ All unsolved in dimension -> if candidate only exists in one node in dimension, it can be solved]
	SUDOKULIB_PUBLIC bool removeHiddenSingle(SudokuContext& p);

	SUDOKULIB_PUBLIC bool removeNakedPair(SudokuContext& p);

	SUDOKULIB_PUBLIC bool removeNakedTriplet(SudokuContext& p);

	// hidden implies that a DIMENSION lacks CANDIDATE, and only a few nodes can have that value, if 2 candidate can only exist in 2 nodes, those values MUST be in those nodes
	SUDOKULIB_PUBLIC bool removeHiddenPair(SudokuContext& p);

	SUDOKULIB_PUBLIC bool removeHiddenTriplet(SudokuContext& p);

	SUDOKULIB_PUBLIC bool removeNakedQuad(SudokuContext& p);

	SUDOKULIB_PUBLIC bool removeHiddenQuad(SudokuContext& p);

	SUDOKULIB_PUBLIC bool removePointingPair(SudokuContext& p);

	SUDOKULIB_PUBLIC bool removeBoxLineReduction(SudokuContext& p);
	
	SUDOKULIB_PUBLIC bool removeSingleChain(SudokuContext& p);

	SUDOKULIB_PUBLIC bool removeXWing(SudokuContext& p);

	SUDOKULIB_PUBLIC bool removeYWing(SudokuContext& p);

	SUDOKULIB_PUBLIC bool removeUniqueRectangle(SudokuContext& p);

	using TechniqueFunction = std::function<bool(SudokuContext& p)>;
	SUDOKULIB_PUBLIC std::vector<TechniqueFunction> allTechniques();
	
} // techniques
