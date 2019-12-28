#pragma once

#include <algorithm>
#include <array>
#include <bitset>

#include <Core/Types.h>
#include <SudokuAlgorithms/Module.h>

namespace dd
{
	const uint BoardSize = 81u;

	struct Candidates
	{
		static const u16 All = 0x3FF;
		static const u16 c1 = 1 << 1;
		static const u16 c2 = 1 << 2;
		static const u16 c3 = 1 << 3;
		static const u16 c4 = 1 << 4;
		static const u16 c5 = 1 << 5;
		static const u16 c6 = 1 << 6;
		static const u16 c7 = 1 << 7;
		static const u16 c8 = 1 << 8;
		static const u16 c9 = 1 << 9;
	};

	struct Node
	{
		using ValueType = u16;
		Node() {
			bits.all = 0u;
		}

		bool operator==(const uint& other) const
		{
			if (other == 0)
				return bits.solved == 0 && bits.value == 0;
			return bits.value == other && bits.solved == 1;
		}

		bool operator==(const Node& other) const
		{
			return bits.all == other.bits.all;
		}

		void solve(u32 value) { bits.all = 0; bits.value = value; bits.solved = 1u; }
		void setCandidatesFromMask(ValueType candidateMask) { bits.candidates = candidateMask; }

		void removeCandidate(ValueType candidate) { 
			ValueType mask = ~(1 << candidate);
			bits.candidates &= mask; 
		}

		bool isEmpty() const { return bits.all == 0; }
		bool isSolved() const { return bits.solved; }
		ValueType getCandidates() const { return bits.candidates; }
		ValueType getValue() const { return bits.value; }

	private:
		union Value
		{
			struct
			{
				ValueType candidates : 10;
				ValueType value : 5;
				ValueType solved : 1;
			};
			ValueType all;
		};

		Value bits;
	};

	struct Board
	{
		Node Nodes[BoardSize];
		char raw[BoardSize];

		Board operator=(const Board& other)
		{
			Board b;
			memcpy(&b.raw[0], &other.raw[0], sizeof(char) * BoardSize);
			memcpy(&b.Nodes[0], &other.Nodes[0], sizeof(Node) * BoardSize);
			return b;
		}

		Node& begin() { return Nodes[0]; }
		Node& end() { return Nodes[81]; }

		bool operator==(const Board& other) const
		{
			for (uint i = 0; i < BoardSize; ++i)
			{
				if ((Nodes[i] == other.Nodes[i]) == false)
					return false;
				if (raw[i] != other.raw[i])
					return false;
			}
			return true;
		}

		bool operator!=(const Board& other) const
		{
			return (*this == other) == false;
		}

		static Board fromString(const char* data) {
			auto charToNode = [](char c) -> Node {
				Node n;
				switch (c)
				{
				case '.':
				case 'x':
				case ' ':
					break;
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					u16 value = c - '0';
					n.solve(value);
					break;
				}
				return n;
			};

			Board b;
			for (uint i = 0; i < BoardSize; ++i)
			{
				b.raw[i] = data[i];
				b.Nodes[i] = charToNode(data[i]);
			}

			return b;
		}
	};

	struct Row
	{
		Node data[9];
	};

	struct Column
	{
		Node data[9];
	};

	struct Cell
	{
		Node data[9];
	};

	Row getRow(const Board& b, uint rowId)
	{
		Row row;
		for (uint i = 0; i < 9; ++i)
			row.data[i] = b.Nodes[i + rowId * 9];

		return row;
	}

	Column getColumn(const Board& b, uint columnId)
	{
		Column col;
		for (uint i = 0; i < 9; ++i)
		{
			col.data[i] = b.Nodes[columnId + (i * 9)];
		}
		return col;
	}

	constexpr u16 topLeftFromCellId(uint cellId)
	{
		u16 rOffset = (cellId / 3);
		u16 cOffset = (cellId % 3);
		return
			27 * rOffset +
			3 * cOffset;
	}

	Cell getCell(const Board& b, uint cellId)
	{
		//	0:	 		1 :			2 :
		//	00 01 02	03 04 05	06 07 08
		//	09 10 11	12 13 14	15 16 17
		//	18 19 20	21 22 23	24 25 26

		//	3:			4 :			5 :
		//	27 28 29
		//	36 37 38	39 40 41	42 43 44
		//	45 46 47

		//	6:
		//	54 55 56
		//	63 64 65
		//  72 73 74

		Cell cell;
		u16 top_left = topLeftFromCellId(cellId);
		for (uint i = 0; i < 3; ++i)
		{
			cell.data[i * 3 + 0] = b.Nodes[top_left + i * 9 + 0];
			cell.data[i * 3 + 1] = b.Nodes[top_left + i * 9 + 1];
			cell.data[i * 3 + 2] = b.Nodes[top_left + i * 9 + 2];
		}
		return cell;
	}


	struct BoardBits
	{
		using SudokuBitBoard = std::bitset<128>;
		using BitBoards9 = std::array<SudokuBitBoard, 9>; // for instance all rows
		using BitBoards3 = std::array<SudokuBitBoard, 3>; // for instance neighbours given a specific node

		static constexpr SudokuBitBoard AllNodes() {
			SudokuBitBoard board{};
			for (auto i = 0; i < BoardSize; ++i)
				board.set(i);
			return board;
		}

		static constexpr SudokuBitBoard EmptyMask() { return SudokuBitBoard{}; }

		static constexpr SudokuBitBoard BitRow(uint rowId) {
			SudokuBitBoard row{};
			for (uint i = 0; i < 9; ++i)
				row.set(i + rowId * 9);
			return row;
		}

		static constexpr SudokuBitBoard BitColumn(uint columnId) {
			SudokuBitBoard col{};
			for (uint i = 0; i < 9; ++i)
				col.set(columnId + (i * 9));
			return col;
		}

		static constexpr SudokuBitBoard BitCell(uint cellId) {
			SudokuBitBoard cell{};
			u16 top_left = topLeftFromCellId(cellId);
			for (uint i = 0; i < 3; ++i)
			{
				cell.set(top_left + i * 9 + 0);
				cell.set(top_left + i * 9 + 1);
				cell.set(top_left + i * 9 + 2);
			}
			return cell;
		}

		//////////////////////////////////////////////////////

		static constexpr BitBoards9 AllRows() {
			BitBoards9 rows; 
			for(uint i=0; i < 9; ++i)
				rows[i] = BitRow(i);
			return rows;
		}

		static constexpr BitBoards9 AllColumns() {
			BitBoards9 columns;
			for (uint i = 0; i < 9; ++i)
				columns[i] = BitColumn(i);
			return columns;
		}

		static constexpr BitBoards9 AllCells() {
			BitBoards9 cells;
			for (uint i = 0; i < 9; ++i)
				cells[i] = BitCell(i);
			return cells;
		}

		static constexpr uint RowForNodeId(uint nodeId) { return nodeId / 9; }
		static constexpr uint ColumnForNodeId(uint nodeId) { return nodeId % 9; }
		static constexpr uint CellForNodeId(uint nodeId) { 
			const uint rowId = RowForNodeId(nodeId);
			const uint columnId = ColumnForNodeId(nodeId);
			const uint rowOffset = (rowId / 3) * 3; // only take full 3's and multiply with 3 [0..2] --> 0, [3..5] --> 3
			const uint colOffset = columnId / 3;
			const uint cellId = rowOffset + colOffset;
			return cellId;
		}

		// is inclusive [contains self(nodeId)], is this correct?
		static constexpr BitBoards3 NeighboursForNode(uint nodeId) {
			const uint rowId = RowForNodeId(nodeId);
			const uint columnId = ColumnForNodeId(nodeId);
			const uint cellId = CellForNodeId(nodeId);

			return BitBoards3{ BitRow(rowId) , BitColumn(columnId), BitCell(cellId) };
		}

		//////////////////////////////////////////////////////

		static SudokuBitBoard bitsSolved(const Board& b)
		{
			SudokuBitBoard bits;
			for (uint i=0; i < BoardSize; ++i)
				bits.set(i, b.Nodes[i].isSolved());
			return bits;
		}

		static SudokuBitBoard bitsUnsolved(const Board& b)
		{
			SudokuBitBoard bits;
			for (uint i = 0; i < BoardSize; ++i)
				bits.set(i, !b.Nodes[i].isSolved());
			return bits;
		}

		using SetBitForNodePredicate = std::function<bool(const Node&)>;
		static SudokuBitBoard bitsPredicate(const Board& b, SetBitForNodePredicate f)
		{
			SudokuBitBoard bits;
			for (uint i = 0; i < BoardSize; ++i)
				bits.set(i, f(b.Nodes[i]));
			return bits;
		}
	};

}