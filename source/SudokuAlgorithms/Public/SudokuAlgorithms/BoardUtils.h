#pragma once

#include <vector>

#include <SudokuAlgorithms/BoardUtils.h>
#include <SudokuAlgorithms/Module.h>
#include <SudokuAlgorithms/SudokuTypes.h>

namespace dd
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

	u8 countCandidates(u16 mask) {
		return static_cast<u8>(__popcnt16(mask));
	}

	u16 candidateIdToMask(u8 candidateId) {
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

		constexpr BitBoards3 NeighboursForNode(uint nodeId) {
			const uint rowId = BoardUtils::RowForNodeId(nodeId);
			const uint columnId = BoardUtils::ColumnForNodeId(nodeId);
			const uint blockId = BoardUtils::BlockForNodeId(nodeId);

			return BitBoards3{ BitRow(rowId) , BitColumn(columnId), BitBlock(blockId) };
		}

		constexpr BitBoard NeighboursForNodeCombined(uint nodeId) {
			const uint rowId = BoardUtils::RowForNodeId(nodeId);
			const uint columnId = BoardUtils::ColumnForNodeId(nodeId);
			const uint blockId = BoardUtils::BlockForNodeId(nodeId);

			BitBoard b = BitRow(rowId) | BitColumn(columnId) | BitBlock(blockId);
			b.clearBit(nodeId);
			return b;
		}

		constexpr BitBoards3 NeighboursForNodeClearSelf(uint nodeId) {
			BitBoards3 boards = NeighboursForNode(nodeId);
			boards[0].clearBit(nodeId);
			boards[1].clearBit(nodeId);
			boards[2].clearBit(nodeId);
			return boards;
		}

		SudokuBitBoard SharedNeighborsClearSelf(const u16* nodes, u8 count) {
			BitBoard sharedNeighbours;

			u32 rows[8];
			u32 cols[8];
			u32 blocks[8];

			for (uint i = 0; i < count; ++i) {
				rows[i] = BoardUtils::RowForNodeId(nodes[i]);
				cols[i] = BoardUtils::ColumnForNodeId(nodes[i]);
				blocks[i] = BoardUtils::BlockForNodeId(nodes[i]);
			}

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

		constexpr SudokuBitBoard SharedNeighboursClearSelf(u32 node1, u32 node2) {
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

		SudokuBitBoard SharedSeenNodes(u8* nodes, u8 numNodes) {
			BitBoard combined(BitBoard::All{});

			for (uint i = 0; i < numNodes; ++i) {
				combined &= NeighboursForNodeCombined(nodes[i]);
			}

			return combined;
		}

		SudokuBitBoard SharedNeighborsClearSelf(const u8* nodes, u8 count) {
			u16 temp[9];
			for (uint i = 0; i < count; ++i)
				temp[i] = nodes[i];

			return SharedNeighborsClearSelf(&temp[0], count);
		}

		//////////////////////////////////////////////////////

		SudokuBitBoard bitsSolved(const Board& b)
		{
			SudokuBitBoard bits;
			for (uint i = 0; i < BoardSize; ++i)
				if (b.Nodes[i].isSolved())
					bits.setBit(i);
			return bits;
		}

		SudokuBitBoard bitsUnsolved(const Board& b)
		{
			SudokuBitBoard bits;
			for (uint i = 0; i < BoardSize; ++i)
				bits.modifyBit(i, !b.Nodes[i].isSolved());
			return bits;
		}

		BitBoards9 buildCandidateBoards(Board& b) {
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

		u8 rowToDimension(u32 i) { return static_cast<u8>(i); }
		u8 colToDimension(u32 i) { return static_cast<u8>(i + 9u); }
		u8 blockToDimension(u32 i) { return static_cast<u8>(i + 18u); }

		struct CandidatesInDimensionBoard {
			BitBoard board;
			u8 numNodes;
			u8 dimensionId;
			u8 candidateId;
			//u8 nodes[9]; // #todo : not sure if this is neccessary 
		};

		using CandidateBoards9 = std::array<CandidatesInDimensionBoard, 9>; // for instance all different rows

		CandidateBoards9 buildAllRowsForCandidate(SudokuContext& p, u8 candidateId) {
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

		CandidateBoards9 buildAllColumnsForCandidate(SudokuContext& p, u8 candidateId) {
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

		bool sharesBlock(u8& outBlockId, const BitBoard& nodes) {
			for (uint i = 0; i < 9; ++i) {
				const BitBoard inDimension = BitBlock(i) & nodes;
				if (inDimension == nodes) {
					outBlockId = static_cast<u8>(i);
					return true;
				}
			}
			return false;
		}

		bool sharesColumn(u8& outColumnId, const BitBoard& nodes) {
			for (uint i = 0; i < 9; ++i) {
				const BitBoard inDimension = BitColumn(i) & nodes;
				if (inDimension == nodes) {
					outColumnId = static_cast<u8>(i);
					return true;
				}
			}
			return false;
		}

		bool sharesRow(u8& outRowId, const BitBoard& nodes) {
			for (uint i = 0; i < 9; ++i) {
				const BitBoard inDimension = BitRow(i) & nodes;
				if (inDimension == nodes) {
					outRowId = static_cast<u8>(i);
					return true;
				}
			}
			return false;
		}
	}

	namespace BoardUtils {
		u16 mergeCandidateMasks(const SudokuContext& p, u8* nodeIds, u8 count) {
			u16 mask = 0;
			for (uint i = 0; i < count; ++i) {
				mask |= p.b.Nodes[nodeIds[i]].getCandidates();
			}
			return mask;
		}

		BitBoard mergeCandidateBoards(const SudokuContext& p, u16 combinedMask) {
			BitBoard merged;
			for (uint i = 0; i < 9; ++i) {
				u16 c = 1 << (i + 1);
				if (combinedMask & c) {
					merged |= p.AllCandidates[i];
				}
			}

			return merged;
		}

		void removeCandidates(SudokuContext& p, u16 candidatesToRemove, const BitBoard& affectedNodes) {
			affectedNodes.foreachSetBit([&p, candidatesToRemove](u32 bitIndex) {
				p.b.Nodes[bitIndex].candidatesRemoveBySolvedMask(candidatesToRemove);
			});
		}

		u16 buildValueMaskFromSolvedNodes(const Node* nodes, const BitBoard& affectedNodes)
		{
			u32 valueMask = 0U;

			affectedNodes.foreachSetBit([&valueMask, nodes](u32 bitIndex) {
				valueMask |= (1u << nodes[bitIndex].getValue());
			});

			return static_cast<u16>(valueMask);
		}

		void removeCandidatesForNodes(Node* nodes, const BitBoard& affectedNodes, u32 savedCandidates) {
			affectedNodes.foreachSetBit([savedCandidates, nodes](u32 bitIndex) {
				nodes[bitIndex].candidatesToKeep(static_cast<u16>(savedCandidates));
			});
		}

		BitBoard boardWhereCandidateCountInRange(const SudokuContext& p, int maxCandidates, int minCandidates = 2) {
			BitBoard potentials;
			for (uint i = 0; i < BoardSize; ++i) {
				Node n = p.b.Nodes[i];
				const u8 numCandidates = countCandidates(n.getCandidates());
				if (numCandidates >= minCandidates && numCandidates <= maxCandidates) {
					potentials.setBit(i);
				}
			}
			return potentials;
		}

		void countTimesEachCandidateOccur(u8* candidateCount, const SudokuContext& p, const BitBoard& affectedNodes) {
			for (uint i = 0; i < 9; ++i) {
				candidateCount[i] = (affectedNodes & p.AllCandidates[i]).countSetBits();
			}
		}
	}

	u16 buildSolvedMask(const SudokuContext& p, u8* nodeIds, u8 count) {
		u16 merged = 0;
		for (uint i = 0; i < count; ++i)
			merged |= 1 << p.b.Nodes[nodeIds[i]].getValue();
		return merged;
	}

	u16 buildSolvedMask(const SudokuContext& p, const BitBoard& affectedNodes) {
		u16 merged = 0;
		affectedNodes.foreachSetBit([&p, &merged](u32 bit) {
			merged |= 1 << p.b.Nodes[bit].getValue();
		});
		return merged;
	}

	u16 buildValueMaskFromCandidateIds(const u16* ids, u32 count) {
		u16 mask = 0;
		for (uint i = 0; i < count; ++i)
			mask |= 1u << (ids[i] + 1); // ids are zero based
		return mask;
	}

	u16 toCandidateMask(u32 valueMask)
	{
		return (~valueMask) & Candidates::All;
	}

	u16 getOnlyCandidateFromMask(u16 candidates) {
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