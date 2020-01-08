#pragma once

#include <vector>

#include <SudokuAlgorithms/BoardUtils.h>
#include <SudokuAlgorithms/Module.h>
#include <SudokuAlgorithms/SudokuTypes.h>

namespace dd
{
	u8 countCandidates(u16 mask) {
		return static_cast<u8>(__popcnt16(mask));
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

		constexpr BitBoards3 NeighboursForNode(uint nodeId) {
			const uint rowId = RowForNodeId(nodeId);
			const uint columnId = ColumnForNodeId(nodeId);
			const uint blockId = BlockForNodeId(nodeId);

			return BitBoards3{ BitRow(rowId) , BitColumn(columnId), BitBlock(blockId) };
		}

		constexpr BitBoard NeighboursForNodeCombined(uint nodeId) {
			const uint rowId = RowForNodeId(nodeId);
			const uint columnId = ColumnForNodeId(nodeId);
			const uint blockId = BlockForNodeId(nodeId);

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

		SudokuBitBoard DistinctNeighboursClearSelf(const u16* nodes, u8 count) {
			BitBoard sharedNeighbours;

			u32 rows[8];
			u32 cols[8];
			u32 blocks[8];

			for (uint i = 0; i < count; ++i) {
				rows[i] = RowForNodeId(nodes[i]);
				cols[i] = ColumnForNodeId(nodes[i]);
				blocks[i] = BlockForNodeId(nodes[i]);
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
			const u32 c1 = ColumnForNodeId(node1);
			const u32 c2 = ColumnForNodeId(node2);
			const u32 r1 = RowForNodeId(node1);
			const u32 r2 = RowForNodeId(node2);
			const u32 b1 = BlockForNodeId(node1);
			const u32 b2 = BlockForNodeId(node2);

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
	}

	namespace BoardUtils {
		BitBoard mergeCandidateBoards(SudokuContext& p, u16 combinedMask) {
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

			if (valueMask & 1u)
				assert(false);
			return static_cast<u16>(valueMask);
		}

		BitBoard wouldRemoveCandidates(Node* nodes, const BitBoard& affectedNodes, u32 possibleCandidateMask) {
			BitBoard modifiedNodes;
			affectedNodes.foreachSetBit([possibleCandidateMask, nodes, &modifiedNodes](u32 bitIndex) {
				u16 currCandidates = nodes[bitIndex].getCandidates();
				if ((possibleCandidateMask & currCandidates) != currCandidates)
					modifiedNodes.setBit(bitIndex);
			});
			return modifiedNodes;
		}

		void removeCandidatesForNodes(Node* nodes, const BitBoard& affectedNodes, u32 savedCandidates) {
			affectedNodes.foreachSetBit([savedCandidates, nodes](u32 bitIndex) {
				nodes[bitIndex].candidatesToKeep(static_cast<u16>(savedCandidates));
			});
		}
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

	
}