#pragma once

#include <SudokuAlgorithms/Module.h>
#include <Core/Types.h>
#include <algorithm>

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

		void solve(ValueType value) { bits.all = 0; bits.value = value; bits.solved = 1u; }
		void setCandidatesFromMask(ValueType candidateMask) { bits.candidates = candidateMask; }

		void removeCandidate(ValueType candidate) { 
			ValueType mask = ~(1 << candidate);
			bits.candidates &= mask; 
		}

		bool isEmpty() { return bits.all == 0; }
		bool isSolved() { return bits.solved; }
		ValueType getCandidates() { return bits.candidates; }
		ValueType getValue() { return bits.value; }

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
		}

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

	u16 topLeftFromCellId(uint cellId)
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
		//	36 37 38
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
}