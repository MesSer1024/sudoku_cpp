#pragma once

#include <vector>

#include <SudokuLib/sudokulib_module.h>
#include <SudokuLib/SudokuTypes.h>

namespace ddahlkvist
{
	namespace BoardUtils {
		constexpr uint RowForNodeId(uint nodeId) { return nodeId / 9; }
		constexpr uint ColumnForNodeId(uint nodeId) { return nodeId % 9; }
		constexpr uint BlockForNodeId(uint nodeId) {
			const uint rowId = RowForNodeId(nodeId);
			const uint columnId = ColumnForNodeId(nodeId);
			const uint rowOffset = (rowId / 3) * 3; // only take full 3's and multiply with 3 [0..2] --> 0, [3..5] --> 3
			const uint colOffset = columnId / 3;
			const uint blockId = rowOffset + colOffset;
			return blockId;
		}

		constexpr bool sharesRow(u8* nodeIds, u8 count) {
			const uint ExpectedDimension = RowForNodeId(nodeIds[0]);
			for (uint i = 1; i < count; ++i) {
				uint dim = RowForNodeId(nodeIds[i]);
				if (dim != ExpectedDimension)
					return false;
			}
			return true;
		}

		constexpr bool sharesColumn(u8* nodeIds, u8 count) {
			const uint ExpectedDimension = ColumnForNodeId(nodeIds[0]);
			for (uint i = 1; i < count; ++i) {
				uint dim = ColumnForNodeId(nodeIds[i]);
				if (dim != ExpectedDimension)
					return false;
			}
			return true;
		}
	}

	inline u8 countBits(u32 data) {
		return static_cast<u8>(__popcnt(data));
	}

	inline u8 countCandidates(u16 with2nodes) {
		return static_cast<u8>(__popcnt16(with2nodes));
	}

	inline u16 candidateIdToMask(u8 candidateId) {
		u16 candidateMask = 1 << (candidateId + 1);
		return candidateMask;
	}

	constexpr u16 topLeftFromblockId(uint blockId)
	{
		u32 rOffset = (blockId / 3);
		u32 cOffset = (blockId % 3);
		return
			static_cast<u16>(27 * rOffset +
				3 * cOffset);
	}

	namespace BoardBits
	{
		constexpr SudokuBitBoard BitRow(uint rowId) {
			SudokuBitBoard row{};
			for (uint i = 0; i < 9; ++i)
				row.setBit(i + rowId * 9);
			return row;
		}

		constexpr SudokuBitBoard BitColumn(uint columnId) {
			SudokuBitBoard col{};
			for (uint i = 0; i < 9; ++i)
				col.setBit(columnId + (i * 9));
			return col;
		}

		constexpr SudokuBitBoard BitBlock(uint blockId) {
			SudokuBitBoard block{};
			u16 top_left = topLeftFromblockId(blockId);
			for (uint i = 0; i < 3; ++i)
			{
				block.setBit(top_left + i * 9 + 0);
				block.setBit(top_left + i * 9 + 1);
				block.setBit(top_left + i * 9 + 2);
			}
			return block;
		}

		//////////////////////////////////////////////////////

		constexpr BitBoards9 AllRows() {
			BitBoards9 rows;
			for (uint i = 0; i < 9; ++i)
				rows[i] = BitRow(i);
			return rows;
		}

		constexpr BitBoards9 AllColumns() {
			BitBoards9 columns;
			for (uint i = 0; i < 9; ++i)
				columns[i] = BitColumn(i);
			return columns;
		}

		constexpr BitBoards9 AllBlocks() {
			BitBoards9 blocks;
			for (uint i = 0; i < 9; ++i)
				blocks[i] = BitBlock(i);
			return blocks;
		}

		constexpr BitBoards27 AllDimensions() {
			BitBoards27 dimensions;
			for (uint i = 0; i < 9; ++i) {
				dimensions[i] = BitRow(i);
				dimensions[i + 9] = BitColumn(i);
				dimensions[i + 18] = BitBlock(i);
			}
			return dimensions;
		}

		constexpr BitBoard NeighboursForNode(uint nodeId) {
			const uint rowId = BoardUtils::RowForNodeId(nodeId);
			const uint columnId = BoardUtils::ColumnForNodeId(nodeId);
			const uint blockId = BoardUtils::BlockForNodeId(nodeId);

			BitBoard b = BitRow(rowId) | BitColumn(columnId) | BitBlock(blockId);
			b.clearBit(nodeId);
			return b;
		}

		inline SudokuBitBoard NeighboursIntersection(u8* nodes, u8 numNodes) {
			BitBoard combined(BitBoard::All{});

			for (uint i = 0; i < numNodes; ++i) {
				combined &= NeighboursForNode(nodes[i]);
			}

			return combined;
		}

		inline SudokuBitBoard NeighboursUnion_ifAllNodesAreSameDimension(const u8* nodes, u8 count) {
			BitBoard sharedNeighbours;

			u32 rows[9];
			u32 cols[9];
			u32 blocks[9];

			for (uint i = 0; i < count; ++i) {
				rows[i] = BoardUtils::RowForNodeId(nodes[i]);
				cols[i] = BoardUtils::ColumnForNodeId(nodes[i]);
				blocks[i] = BoardUtils::BlockForNodeId(nodes[i]);
			}

			// #todo : is this correct? Doesn't adjacent_find require data to be sorted?
			if (std::adjacent_find(rows, rows + count, std::not_equal_to<>()) == rows + count)
				sharedNeighbours |= BitRow(rows[0]);
			if (std::adjacent_find(cols, cols + count, std::not_equal_to<>()) == cols + count)
				sharedNeighbours |= BitColumn(cols[0]);
			if (std::adjacent_find(blocks, blocks + count, std::not_equal_to<>()) == blocks + count)
				sharedNeighbours |= BitBlock(blocks[0]);

			for (uint i = 0; i < count; ++i) {
				sharedNeighbours.clearBit(nodes[i]);
			}

			return sharedNeighbours;
		}

		constexpr SudokuBitBoard NeighboursUnion_ifNodesAreSameDimension(u32 node1, u32 node2) {
			const u32 c1 = BoardUtils::ColumnForNodeId(node1);
			const u32 c2 = BoardUtils::ColumnForNodeId(node2);
			const u32 r1 = BoardUtils::RowForNodeId(node1);
			const u32 r2 = BoardUtils::RowForNodeId(node2);
			const u32 b1 = BoardUtils::BlockForNodeId(node1);
			const u32 b2 = BoardUtils::BlockForNodeId(node2);

			BitBoard sharedNeighbours;

			if (c1 == c2)
				sharedNeighbours |= BitColumn(c1);
			if (r1 == r2)
				sharedNeighbours |= BitRow(r1);
			if (b1 == b2)
				sharedNeighbours |= BitBlock(b1);

			sharedNeighbours.clearBit(node1);
			sharedNeighbours.clearBit(node2);

			return sharedNeighbours;
		}

		//////////////////////////////////////////////////////

		inline SudokuBitBoard bitsSolved(const Board& b)
		{
			SudokuBitBoard bits;
			for (uint i = 0; i < BoardSize; ++i)
				if (b.Nodes[i].isSolved())
					bits.setBit(i);
			return bits;
		}

		inline SudokuBitBoard bitsUnsolved(const Board& b)
		{
			SudokuBitBoard bits;
			for (uint i = 0; i < BoardSize; ++i)
				bits.modifyBit(i, !b.Nodes[i].isSolved());
			return bits;
		}

		inline BitBoards9 buildCandidateBoards(Board& b) {
			BitBoards9 out;

			for (uint i = 0; i < BoardSize; ++i) {
				if (!b.Nodes[i].isSolved()) {
					u16 candidate = b.Nodes[i].getCandidates();
					for (u16 c = 0; c < 9; ++c) {
						if (candidate & AllCandidatesArray[c]) {
							out[c].setBit(i);
						}
					}
				}
			}

			return out;
		}

		inline u8 rowToDimension(u32 i) { return static_cast<u8>(i); }
		inline u8 colToDimension(u32 i) { return static_cast<u8>(i + 9u); }
		inline u8 blockToDimension(u32 i) { return static_cast<u8>(i + 18u); }

		struct CandidatesInDimensionBoard {
			BitBoard board;
			u8 numNodes;
			u8 dimensionId;
			u8 candidateId;
			//u8 nodes[9]; // #todo : not sure if this is neccessary 
		};

		using CandidateBoards9 = std::array<CandidatesInDimensionBoard, 9>; // for instance all different rows

		inline CandidateBoards9 buildAllRowsForCandidate(SudokuContext& p, u8 candidateId) {
			CandidateBoards9 out;
			for (u8 i = 0; i < 9; ++i) {
				CandidatesInDimensionBoard& ref = out[i];
				ref.board = p.AllCandidates[candidateId] & BoardBits::BitRow(i);
				ref.candidateId = candidateId;
				ref.dimensionId = rowToDimension(i);
				//ref.numNodes = ref.board.fillSetBits(ref.nodes);
				ref.numNodes = ref.board.countSetBits();
			}
			return out;
		}

		inline CandidateBoards9 buildAllColumnsForCandidate(SudokuContext& p, u8 candidateId) {
			CandidateBoards9 out;
			for (u8 i = 0; i < 9; ++i) {
				CandidatesInDimensionBoard& ref = out[i];
				ref.board = p.AllCandidates[candidateId] & BoardBits::BitColumn(i);
				ref.candidateId = candidateId;
				ref.dimensionId = colToDimension(i);
				//ref.numNodes = ref.board.fillSetBits(ref.nodes);
				ref.numNodes = ref.board.countSetBits();
			}
			return out;
		}

		inline bool sharesBlock(u8& outBlockId, const BitBoard& nodes) {
			for (uint i = 0; i < 9; ++i) {
				const BitBoard inDimension = BitBlock(i) & nodes;
				if (inDimension == nodes) {
					outBlockId = static_cast<u8>(i);
					return true;
				}
			}
			return false;
		}

		inline bool sharesColumn(u8& outColumnId, const BitBoard& nodes) {
			for (uint i = 0; i < 9; ++i) {
				const BitBoard inDimension = BitColumn(i) & nodes;
				if (inDimension == nodes) {
					outColumnId = static_cast<u8>(i);
					return true;
				}
			}
			return false;
		}

		inline bool sharesRow(u8& outRowId, const BitBoard& nodes) {
			for (uint i = 0; i < 9; ++i) {
				const BitBoard inDimension = BitRow(i) & nodes;
				if (inDimension == nodes) {
					outRowId = static_cast<u8>(i);
					return true;
				}
			}
			return false;
		}

		// If a candidate appears EXACTLY twice in a unit, then those two candidates are called a conjugate pair.
		struct ConjugatePair {
			u8 node1;
			u8 node2;
			u8 candidateId;
			u8 dimensionId;
		};

		// If a candidate appears EXACTLY twice in a unit, then those two candidates are called a conjugate pair.
		inline std::vector<ConjugatePair> findConjugatePairsForCandidate(const SudokuContext& p, u8 candidateId) {
			std::vector<ConjugatePair> pairs;

			const BitBoard& candidates = p.AllCandidates[candidateId];
			for (u8 i = 0; i < 27; ++i) {
				const BitBoard inDimension = p.AllDimensions[i] & candidates;
				const u8 numOccurances = inDimension.countSetBits();
				if (numOccurances == 2) {
					u8 nodes[2];
					inDimension.fillSetBits(nodes);

					pairs.push_back(ConjugatePair{ nodes[0], nodes[1], candidateId, i });
				}
			}

			return pairs;
		}

		// #todo - performance measure this vs nodesWithCandidateCountBetweenXY(2,2)
		//BitBoard nodesWithExactlyTwoCandidates(const BoardBits::BitBoards9& candidateBoards) {
		//	BitBoard invalidated;
		//	BitBoard hitOnce;
		//	BitBoard hitTwice;

		//	for (uint i = 0; i < 9; ++i) {
		//		BitBoard overlap = candidateBoards[i] & hitOnce;
		//		if (overlap.notEmpty()) {
		//			invalidated |= (overlap & hitTwice); //
		//			hitTwice |= overlap;
		//		}
		//		hitOnce |= candidateBoards[i];
		//	}

		//	return hitTwice & (invalidated.invert());
		//}

		inline BitBoard nodesWithExactlyTwoCandidates(const BoardBits::BitBoards9& candidateBoards) {
			// Algorithm description: we iterate over each "candidate board"
			// each candidate board has a bit set if a node consider this candidate to be a possibility
			// if we store what node was previously hit we can then deduct if a Node has been hit twice
			// if we then track what nodes has been hit "twice", if they happen to be hit an additional time, that means that they are invalidated...

			BitBoard invalidated;
			BitBoard hitOnce;
			BitBoard hitTwice;

			for (const auto& nodesWithCandidateX : candidateBoards) {
				BitBoard overlap = nodesWithCandidateX & hitOnce;
				if (overlap.notEmpty()) {
					invalidated |= (overlap & hitTwice); //
					hitTwice |= overlap;
				}
				hitOnce |= nodesWithCandidateX;
			}

			return hitTwice & (invalidated.invert());
		}
		 
		inline BitBoard nodesWithCandidateCountBetweenXY(const BoardBits::BitBoards9& candidateBoards, int min, int max) {
			assert(min != 0);
			assert(max < 9);
			// Algorithm description: We iterate over each candidate board [each bit indicates THAT NODE consider candidateX a possibility]
			// We track all nodes being hit at least once, at least twice, at least thrice etc...
			// Then --> Each node previously hit 3 times + hit --> hit 4 times
			// Then --> Each node previously hit 2 times + hit --> hit 3 times
			// by appending to "hit_3_times_mask" we make sure that the aggregated result is correct
			// iteration order of "top to bottom" solves the counting

			BitBoard nodesBeingHitAtLeastXTimes[9];

			for (auto& nodesWithCandidateX : candidateBoards) {
				// update how many times each node has been populated with a candidate by iterating from "most" and comparing to "most-1"
				// for correctness we should go from 9 --> 0, but since we know that we do not need to track orders above the treshold [max] we can just ignore any higher values
				for (uint i = max; i > 0; --i)
				{
					BitBoard& nodesWithCandidateCount_i = nodesBeingHitAtLeastXTimes[i];
					BitBoard& nodesWithPrevCandidateCount = nodesBeingHitAtLeastXTimes[i-1];

					const BitBoard newNodesWithCount_i = nodesWithPrevCandidateCount & nodesWithCandidateX;
					nodesWithCandidateCount_i |= newNodesWithCount_i;
				}

				// make sure all nodes that have a candidate are marked as having at least one candidate
				nodesBeingHitAtLeastXTimes[0] |= nodesWithCandidateX;
			}

			// at this point we know exactly how many times each node has been "hit"
			// now we just need to find everyone that has been hit in the range min, max
			// this is most easily done by aggregating results in range [min, max] and then remove the ones found in "max+1"
			BitBoard potentials;
			for (int i = min; i <= max; ++i)
			{
				potentials |= nodesBeingHitAtLeastXTimes[i-1];
			}

			// clear out anyone being hit more than "max times" [the array is 0-based so idx=0 are the ones that have been hit 1 time]
			potentials &= nodesBeingHitAtLeastXTimes[max].invert();

			return potentials;
		}
	}

	/////////////////////////////////////////////////////////////////////////////////

	namespace BoardUtils {
		inline u16 mergeCandidateMasks(const SudokuContext& p, u8* nodeIds, u8 count) {
			u16 with2nodes = 0;
			for (uint i = 0; i < count; ++i) {
				with2nodes |= p.b.Nodes[nodeIds[i]].getCandidates();
			}
			return with2nodes;
		}

		inline BitBoard mergeCandidateBoards(const SudokuContext& p, u16 combinedMask) {
			BitBoard merged;
			for (uint i = 0; i < 9; ++i) {
				u16 c = 1 << (i + 1);
				if (combinedMask & c) {
					merged |= p.AllCandidates[i];
				}
			}

			return merged;
		}

		inline void removeCandidates(SudokuContext& p, u16 candidatesToRemove, const BitBoard& affectedNodes) {
			affectedNodes.foreachSetBit([&p, candidatesToRemove](u32 bitIndex) {
				p.b.Nodes[bitIndex].candidatesRemoveBySolvedMask(candidatesToRemove);
			});
		}

		inline u16 buildValueMaskFromSolvedNodes(const Node* nodes, const BitBoard& affectedNodes)
		{
			u32 valueMask = 0U;

			affectedNodes.foreachSetBit([&valueMask, nodes](u32 bitIndex) {
				valueMask |= (1u << nodes[bitIndex].getValue());
			});

			return static_cast<u16>(valueMask);
		}

		inline void removeCandidatesForNodes(Node* nodes, const BitBoard& affectedNodes, u32 savedCandidates) {
			affectedNodes.foreachSetBit([savedCandidates, nodes](u32 bitIndex) {
				nodes[bitIndex].candidatesToKeep(static_cast<u16>(savedCandidates));
			});
		}

		// #todo : measure performance impact on this vs "inRange"
		//BitBoard boardWhereCandidateCountInRange(const SudokuContext& p, int maxCandidates, int minCandidates = 2) {
		//	BitBoard potentials;
		//	for (uint i = 0; i < BoardSize; ++i) {
		//		Node n = p.b.Nodes[i];
		//		const u8 numCandidates = countCandidates(n.getCandidates());
		//		if (numCandidates >= minCandidates && numCandidates <= maxCandidates) {
		//			potentials.setBit(i);
		//		}
		//	}
		//	return potentials;
		//}

		inline void countTimesEachCandidateOccur(u8* candidateCount, const SudokuContext& p, const BitBoard& affectedNodes) {
			for (uint i = 0; i < 9; ++i) {
				candidateCount[i] = (affectedNodes & p.AllCandidates[i]).countSetBits();
			}
		}
	}

	inline u16 buildSolvedMask(const SudokuContext& p, u8* nodeIds, u8 count) {
		u16 merged = 0;
		for (uint i = 0; i < count; ++i)
			merged |= 1 << p.b.Nodes[nodeIds[i]].getValue();
		return merged;
	}

	inline u16 buildSolvedMask(const SudokuContext& p, const BitBoard& affectedNodes) {
		u16 merged = 0;
		affectedNodes.foreachSetBit([&p, &merged](u32 bit) {
			merged |= 1 << p.b.Nodes[bit].getValue();
		});
		return merged;
	}

	inline u16 buildValueMaskFromCandidateIds(const u16* ids, u32 count) {
		u16 with2nodes = 0;
		for (uint i = 0; i < count; ++i)
			with2nodes |= 1u << (ids[i] + 1); // ids are zero based
		return with2nodes;
	}

	inline u16 toCandidateMask(u32 valueMask)
	{
		return (~valueMask) & Candidates::All;
	}

	inline u16 getOnlyCandidateFromMask(u16 candidates) {
		switch (candidates) {
		case Candidates::c1:
			return 1;
		case Candidates::c2:
			return 2;
		case Candidates::c3:
			return 3;
		case Candidates::c4:
			return 4;
		case Candidates::c5:
			return 5;
		case Candidates::c6:
			return 6;
		case Candidates::c7:
			return 7;
		case Candidates::c8:
			return 8;
		case Candidates::c9:
			return 9;
		}
		assert(false);
		return 0;
	}

	using CandidateBoard = BoardBits::CandidatesInDimensionBoard;
	using CandidateBoards9 = BoardBits::CandidateBoards9;
}