#pragma once

#include <algorithm>
#include <array>
#include <bitset>
#include <functional>
#include <intrin.h>

#include <Core/Types.h>
#include <SudokuAlgorithms/Module.h>

namespace dd
{
	constexpr uint BoardSize = 81u;
	using BitAction = std::function<void(u32 bitIndex)>;

	struct Candidates
	{
		static const u16 All = 0x3FE;
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

	constexpr u16 AllCandidatesArray[9] = { Candidates::c1, Candidates::c2, Candidates::c3, Candidates::c4, Candidates::c5, Candidates::c6, Candidates::c7, Candidates::c8, Candidates::c9 };


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

		void candidatesSet(ValueType candidateMask) { bits.candidates = candidateMask; }
		void candidatesRemoveSingle(ValueType candidate) { 
			ValueType mask = ~(1 << candidate);
			bits.candidates &= mask; 
		}

		void candidatesRemoveBySolvedMask(ValueType solvedMask) {
			bits.candidates = (~solvedMask) & bits.candidates;
		}

		void candidatesToKeep(ValueType possibleCandidates) {
			bits.candidates = possibleCandidates & bits.candidates;
		}

		bool notEmpty() const { return bits.all == 0; }
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

	constexpr u16 topLeftFromblockId(uint blockId)
	{
		u32 rOffset = (blockId / 3);
		u32 cOffset = (blockId % 3);
		return
			static_cast<u16>(27 * rOffset +
			3 * cOffset);
	}

	struct BitBoard
	{
	private:
		u64 bits[2];
		static constexpr u64 LowerMask = ~0ULL;
		static constexpr u64 UpperMask = ((1 << (BoardSize - 64)) - 1);

	public:
		struct None {};
		struct All {};
		constexpr explicit BitBoard(None) : bits{} {}
		constexpr explicit BitBoard(All) : bits{LowerMask, UpperMask} {}
		constexpr BitBoard() : BitBoard(None{}) {}
		constexpr BitBoard(u64 lower, u64 upper) : bits{ lower, upper } {}

		void modifyBit(uint bitIndex, bool flag) {
			if (flag)
				setBit(bitIndex);
			else
				clearBit(bitIndex);
		}

		void setBit(uint bitIndex) { 
#ifdef VALIDATE_BIT_BOUNDS
			assert(bitIndex < BoardSize);
#endif
			const u8 arrIdx = bitIndex >= 64 ? 1 : 0;
			bitIndex = bitIndex % 64;
			bits[arrIdx] |= (1ULL << bitIndex);
		}

		constexpr void clearBit(uint bitIndex) {
#ifdef VALIDATE_BIT_BOUNDS
			assert(bitIndex < BoardSize);
#endif
			const u8 arrIdx = bitIndex >= 64 ? 1 : 0;
			bitIndex = bitIndex % 64;
			bits[arrIdx] &= ~(1ULL << bitIndex);
		}

		bool test(uint bitIndex) const {
			const u8 arrIdx = bitIndex >= 64 ? 1 : 0;
			bitIndex = bitIndex % 64;
			const u64 mask = 1ULL << bitIndex;
			const bool flag = bits[arrIdx] & mask;
			return flag;
		}

		u8 firstOne() const {
			u32 value;
			if (!_BitScanForward64(&value, bits[0]))
			{
				if (!_BitScanForward64(&value, bits[1]))
					return 128;
				else
					value += 64;
			}
			return static_cast<u8>(value);
		}

		constexpr u8 fillSetBits(u8* bitArr) const {
			u8 count = 0;
			for (auto i = 0; i < 64; ++i)
			{
				const u64 testBit = 1ULL << i;
				if (bits[0] & testBit)
					bitArr[count++] = i;
			}
			for (auto i = 0; i < 128 - BoardSize; ++i)
			{
				const u64 testBit = 1ULL << i;
				if (bits[1] & testBit)
					bitArr[count++] = 64 + i;
			}
			return count;
		}

		void foreachSetBit(BitAction action) const {
			u8 bitArr[BoardSize];
			u8 count = fillSetBits(bitArr);

			for (auto i = 0; i < count; ++i)
				action(bitArr[i]);
		}

		bool notEmpty() const {
			return (bits[0] | bits[1]) != 0;
		}

		u32 count() const {
			u64 setBits = __popcnt64(bits[0]);
			setBits += __popcnt64(bits[1]);
			return static_cast<u32>(setBits);
		}

		bool operator==(const BitBoard& other)
		{
			return other.bits[0] == this->bits[0] && other.bits[1] == this->bits[1];
		}

		BitBoard getInverted() const {
			BitBoard inverted;
			inverted.bits[0] = ~bits[0];
			inverted.bits[1] = ~bits[1];
			inverted.bits[1] &= UpperMask;
			return inverted;
		}

		constexpr BitBoard operator&(const BitBoard& other) const {
			BitBoard out;
			out.bits[0] = this->bits[0] & other.bits[0];
			out.bits[1] = this->bits[1] & other.bits[1];
			return out;
		}

		constexpr BitBoard operator|(const BitBoard& other) const {
			BitBoard out;
			out.bits[0] = this->bits[0] | other.bits[0];
			out.bits[1] = this->bits[1] | other.bits[1];
			return out;
		}

		constexpr BitBoard operator^(const BitBoard& other) const {
			BitBoard out;
			out.bits[0] = this->bits[0] ^ other.bits[0];
			out.bits[1] = this->bits[1] ^ other.bits[1];
			return out;
		}

		constexpr void operator|=(const BitBoard& other) {
			this->bits[0] |= other.bits[0];
			this->bits[1] |= other.bits[1];
		}

		constexpr void operator&=(const BitBoard& other) {
			this->bits[0] &= other.bits[0];
			this->bits[1] &= other.bits[1];
		}

		constexpr void operator^=(const BitBoard& other) {
			this->bits[0] ^= other.bits[0];
			this->bits[1] ^= other.bits[1];
		}
	};

	struct BoardBits
	{
		using SudokuBitBoard = BitBoard;
		using BitBoards3 = std::array<SudokuBitBoard, 3>; // for instance neighbours given a specific node
		using BitBoards9 = std::array<SudokuBitBoard, 9>; // for instance all different rows
		using BitBoards27 = std::array<SudokuBitBoard, 27>; // for instance all different rows

		static constexpr SudokuBitBoard BitRow(uint rowId) {
			SudokuBitBoard row{};
			for (uint i = 0; i < 9; ++i)
				row.setBit(i + rowId * 9);
			return row;
		}

		static constexpr SudokuBitBoard BitColumn(uint columnId) {
			SudokuBitBoard col{};
			for (uint i = 0; i < 9; ++i)
				col.setBit(columnId + (i * 9));
			return col;
		}

		static constexpr SudokuBitBoard BitBlock(uint blockId) {
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

		static constexpr BitBoards9 AllBlocks() {
			BitBoards9 blocks;
			for (uint i = 0; i < 9; ++i)
				blocks[i] = BitBlock(i);
			return blocks;
		}

		static constexpr BitBoards27 AllDimensions() {
			BitBoards27 dimensions;
			for (uint i = 0; i < 9; ++i) {
				dimensions[i] = BitRow(i);
				dimensions[i + 9] = BitColumn(i);
				dimensions[i + 18] = BitBlock(i);
			}
			return dimensions;
		}

		static constexpr uint RowForNodeId(uint nodeId) { return nodeId / 9; }
		static constexpr uint ColumnForNodeId(uint nodeId) { return nodeId % 9; }
		static constexpr uint BlockForNodeId(uint nodeId) { 
			const uint rowId = RowForNodeId(nodeId);
			const uint columnId = ColumnForNodeId(nodeId);
			const uint rowOffset = (rowId / 3) * 3; // only take full 3's and multiply with 3 [0..2] --> 0, [3..5] --> 3
			const uint colOffset = columnId / 3;
			const uint blockId = rowOffset + colOffset;
			return blockId;
		}

		static constexpr BitBoard NeighboursForNodeCombined(uint nodeId) {
			const uint rowId = RowForNodeId(nodeId);
			const uint columnId = ColumnForNodeId(nodeId);
			const uint blockId = BlockForNodeId(nodeId);

			BitBoard b = BitRow(rowId) | BitColumn(columnId) | BitBlock(blockId);
			b.clearBit(nodeId);
			return b;
		}

		static constexpr BitBoards3 NeighboursForNodeClearSelf(uint nodeId) {
			BitBoards3 boards = NeighboursForNode(nodeId);
			boards[0].clearBit(nodeId);
			boards[1].clearBit(nodeId);
			boards[2].clearBit(nodeId);
			return boards;
		}

		static constexpr BitBoards3 NeighboursForNode(uint nodeId) {
			const uint rowId = RowForNodeId(nodeId);
			const uint columnId = ColumnForNodeId(nodeId);
			const uint blockId = BlockForNodeId(nodeId);

			return BitBoards3 { BitRow(rowId) , BitColumn(columnId), BitBlock(blockId) };
		}

		static constexpr SudokuBitBoard SharedNeighboursClearSelf(uint node1, uint node2) {
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

		static SudokuBitBoard bitsSolved(const Board& b)
		{
			SudokuBitBoard bits;
			for (uint i = 0; i < BoardSize; ++i)
				if (b.Nodes[i].isSolved())
					bits.setBit(i);
			return bits;
		}

		static SudokuBitBoard bitsUnsolved(const Board& b)
		{
			SudokuBitBoard bits;
			for (uint i = 0; i < BoardSize; ++i)
				bits.modifyBit(i, !b.Nodes[i].isSolved());
			return bits;
		}

		using SetBitForNodePredicate = std::function<bool(const Node&)>;
		static SudokuBitBoard bitsPredicate(const Board& b, SetBitForNodePredicate f)
		{
			SudokuBitBoard bits;
			for (uint i = 0; i < BoardSize; ++i)
				bits.modifyBit(i, f(b.Nodes[i]));
			return bits;
		}
	};

}