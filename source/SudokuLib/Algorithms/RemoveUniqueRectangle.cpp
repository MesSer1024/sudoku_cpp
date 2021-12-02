#pragma once

#include <SudokuLib/SudokuAlgorithm.h>
#include <SudokuLib/TechniqueMeta.h>
#include <BoardUtils.h>

namespace ddahlkvist::techniques
{


	struct DimensionQueryResult {
		// only first 27 bits are used, bits { rows [0..8], cols[9..17], blocks[18..26] }
		u32 nodeCountIs1{};
		u32 nodeCountIs2{};

		static constexpr u32 RowBits = 0x1FF;
		static constexpr u32 ColumnBits = 0x3FE00;
		static constexpr u32 BlockBits = 0x7FC0000;
	};

	DimensionQueryResult findDimensionsWhereNodeCountEquals1or2nodes(const SudokuContext& p, const BitBoard& board) {
		DimensionQueryResult result;

		for (uint i = 0; i < 27; ++i) {
			const BitBoard inDimension = p.AllDimensions[i] & board;
			const u32 numBits = inDimension.countSetBits();
			if (numBits == 1) {
				result.nodeCountIs1 |= 1 << i;
			}
			else if (numBits == 2) {
				result.nodeCountIs2 |= 1 << i;
			}
		}

		return result;
	}

	u8 countColumnsRequired(const SudokuContext& p, const BitBoard& board) {
		u8 count = 0;
		const u8 begin = 9;
		const u8 end = 18;
		for (uint i = begin; i < end; ++i) {
			const BitBoard b = board & p.AllDimensions[i];
			count += b.notEmpty();
		}

		return count;
	}

	void rectangle_foobar(const SudokuContext& p, const BitBoard& board) {
		//struct RectangleMatch {

		//};

		//RectangleMatch matches[100];
		//u8 numMatches = 0;

		//DimensionQueryResult possibleDimensions = findDimensionsWhereNodeCountEquals1or2nodes(p, board);
		//// rows 123 v 789 || 456 v 789 || 123 v 456
		//// 

		//// how can we know that we have 3 nodes that are in a good position? after that, how can we locate the 4th node?
		//	// "any 3 nodes" where exactly 2 nodes share row, and exactly 2 nodes share column
		//	// the 4th node would then be the node that would make it so "any 4 nodes" where exactly 2 nodes share row and exactly 2 nodes share column

		//
		//const auto possibleRows = (possibleDimensions.nodeCountIs1 | possibleDimensions.nodeCountIs2) & possibleDimensions.RowBits;
		//u8 rows[15];
		//const u8 rowCount = fillSetBits(&possibleRows, rows);
		//for (uint i = 0; i < rowCount; ++i) {
		//	const u8 row1 = rows[i];
		//	const bool i_hasOneNode = testBit(possibleDimensions.nodeCountIs1, row1);
		//	const BitBoard row1Nodes = p.AllDimensions[row1] & board;

		//	for (uint j = 0; j < rowCount; ++j) {
		//		const u8 row2 = rows[i];
		//		const bool j_hasOneNode = testBit(possibleDimensions.nodeCountIs2, row2);
		//		
		//		const bool sameBlock = (row1 / 3) == (row2 / 3);
		//		const bool sameNodeCount = i_hasOneNode == j_hasOneNode;
		//		
		//		if (sameBlock || sameNodeCount)
		//			continue;

		//		const BitBoard row2Nodes = p.AllDimensions[row2] & board;
		//		BoardBits::sharesColumn
		//	}
		//}
	}

	bool removeUniqueRectangle(SudokuContext& p) {
		p.result.Technique = Techniques::UniqueRectangle;

		// if we have 4 nodes, forming a rectangle, over 2 rows AND 2 columns AND 2 blocks [with 2 candidates]
		// if that is the case, we would actually have two possible solutions to the puzzle
		// this is not how any published sudoku is meant to function, meaning we can use this technique to our advantage 
		// so if we know what we are looking for, we can make sure we avoid putting us in that direciton by:
		// any "rectangle where candidates 23 appear in 4 nodes, but one corner has an extra candidate", we know that that corner MUST be that extra candidate

		const BitBoard nodesWith2Candidates = BoardBits::nodesWithCandidateCountBetweenXY(p.AllCandidates, 2, 2);
		rectangle_foobar(p, nodesWith2Candidates);
		return p.result.size() > 0;
	}
}