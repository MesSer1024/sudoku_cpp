#pragma once

#include <algorithm>
#include <array>
#include <functional>
#include <intrin.h>
#include <map>
#include <assert.h>

#include <Core/Types.h>
#include <SudokuLib/sudokulib_module.h>

namespace ddahlkvist
{
	enum class Techniques;
	struct Result;

	constexpr uint BoardSize = 81u;

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

		void candidatesSet(ValueType combinedMask) { bits.candidates = combinedMask; }
		void candidatesRemoveSingle(ValueType candidate) { 
			ValueType with2nodes = ~(1 << candidate);
			bits.candidates &= with2nodes; 
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
		u8 fillCandidateIds(u8* candidateIds) const {
			u8 numCandidates = 0;
			u16 candidates = bits.candidates;
			for (u8 i = 0; i < 9; ++i) {
				if (candidates & AllCandidatesArray[i])
					candidateIds[numCandidates++] = i;
			}
			return numCandidates;
		}

		ValueType getValue() const { return bits.value; }

	private:
		#pragma warning(push)
		#pragma warning(disable: 4201)
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
		
		#pragma warning(pop)
	};

	struct Board
	{
		static constexpr u8 MaxPrettyChars = 142;
		Node Nodes[BoardSize];
		char raw[BoardSize];
		char pretty[MaxPrettyChars];

		void updateDebugPretty() {
			auto nodeToChar = [](Node& n) -> char {
				if (n.isSolved()) {
					return '0' + static_cast<char>(n.getValue());
				}
				else {
					return '.';
				}
			};

			uint p = 0;
			for (uint i = 0; i < BoardSize; ++i, ++p) {
				if (i % 27 == 0) {
					if (i != 0) {
						pretty[p++] = '|';
						pretty[p++] = '\n';
						for (uint j = 0; j < 12; ++j) {
							pretty[p++] = '-';
						}
						pretty[p++] = '\n';
					}
				} else if (i % 9 == 0) {
					pretty[p++] = '|';
					pretty[p++] = '\n';
				} else if (i % 3 == 0)
					pretty[p++] = '|';
				pretty[p] = nodeToChar(Nodes[i]);
			}
			pretty[p] = '|';
			assert(p < MaxPrettyChars);
		}

		Board() = default;

		Board(const Board& other)
		{
			memcpy(this->raw, &other.raw[0], sizeof(char) * BoardSize);
			memcpy(this->Nodes, &other.Nodes[0], sizeof(Node) * BoardSize);
		}

		void operator=(const Board& other)
		{
			memcpy(this->raw, &other.raw[0], sizeof(char) * BoardSize);
			memcpy(this->Nodes, &other.Nodes[0], sizeof(Node) * BoardSize);
		}

		//Node& begin() { return Nodes[0]; }
		//Node& end() { return Nodes[81]; }

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
				case '0':
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
		constexpr explicit BitBoard(All) : bits{ LowerMask, UpperMask } {}
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
			const u64 with2nodes = 1ULL << bitIndex;
			const bool flag = bits[arrIdx] & with2nodes;
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

		constexpr inline u8 fillSetBits(u8* __restrict bitArr) const {
			u8* dest = bitArr;

			u64 word1 = bits[0];
			u64 word2 = bits[1];

			for (u8 i = 0u; word1 != 0u; ++i) {
				*dest = i;
				dest += word1 & u64(1u);
				word1 >>= 1u;
			}

			for (u8 i = 64; word2 != 0u; ++i) {
				*dest = i;
				dest += word2 & u64(1u);
				word2 >>= 1u;
			}

			return static_cast<u8>(dest - bitArr);
		}

		template<typename BitAction>
		void foreachSetBit(BitAction&& action) const {
			u8 bitArr[BoardSize + 1]; // need one extra for overwrite protection #todo : write test if this is necessary

			const u8 end = fillSetBits(bitArr);
			for (u8 i = 0; i < end; ++i) {
				action(bitArr[i]);
			}
		}

		bool notEmpty() const {
			return (bits[0] | bits[1]) != 0;
		}

		u8 countSetBits() const {
			u64 numSetBits = __popcnt64(bits[0]);
			numSetBits += __popcnt64(bits[1]);
			return static_cast<u8>(numSetBits);
		}

		bool operator==(const BitBoard& other) const
		{
			return other.bits[0] == this->bits[0] && other.bits[1] == this->bits[1];
		}

		BitBoard invert() const {
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

	struct Change
	{
		u8 index;
		Node prev;
	};

	namespace BoardBits {
		using SudokuBitBoard = BitBoard;
		using BitBoards3 = std::array<SudokuBitBoard, 3>; // for instance neighbours given a specific node
		using BitBoards9 = std::array<SudokuBitBoard, 9>; // for instance all different rows
		using BitBoards27 = std::array<SudokuBitBoard, 27>; // for instance all different rows
	}

	template<typename PtrType>
	struct Span {
		Span(const PtrType* firstElement, u32 count)
			: first(firstElement)
			, last(firstElement+count)
		{}

		const PtrType* begin() const { return first; }
		const PtrType* end() const { return last; }
		u32 size() const { return (last - first); }

	private:
		const PtrType* first;
		const PtrType* last;
	};

	struct SudokuContext {
		Board& b;
		Result& result;

		BoardBits::BitBoards9 SolvedValues;

		BitBoard Solved;		// TODO: Remove
		BitBoard Unsolved;	// TODO: Remove
		BoardBits::BitBoards9 AllCandidates;
		BoardBits::BitBoards27 AllDimensions;
		inline Span<BitBoard> getBlocks() { return Span<BitBoard>(&AllDimensions[18], 9); }
	};

	struct SolveLedger
	{
		static constexpr u32 MaxEntries = 1000;
		
		u32 numIterations = 0;

		Techniques techniqueUsedInIteration[MaxEntries] = {};
		u8 numNodesChangedInIteration[MaxEntries] = {};
	};

	struct Result
	{
		void storePreModification(const Node* nodes, const BitBoard& affectedNodes)
		{
			BitBoard newDirty = (affectedNodes ^ _dirty) & affectedNodes;
			_dirty |= affectedNodes;

#ifdef DD_FINAL
			newDirty.foreachSetBit([&](u32 bitIndex) {
				ChangeCount++;
			});
#else
			newDirty.foreachSetBit([nodes, &Changes = Changes](u32 bitIndex) {
				Changes.push_back({ static_cast<u8>(bitIndex), nodes[bitIndex] });
			});
#endif
		}

		void append(Node old, u8 id)
		{
			if (!_dirty.test(id))
#ifdef DD_FINAL
				ChangeCount++;
#else
				Changes.push_back({ id, old });
#endif

		}

		Change fetch(uint idx)
		{
#ifdef DD_FINAL
			return Change{}; // dunno only used in teset?
#else
			return Changes[idx];
#endif

		}

		uint size() {
#ifdef DD_FINAL
			return ChangeCount;
#else
			return static_cast<uint>(Changes.size());
#endif
		}

		void reset() {
#ifdef DD_FINAL
			ChangeCount = 0;
#else
			Changes.clear();
#endif
			_dirty = {};
			Technique = {};
		}

		Techniques Technique{ };
		SolveLedger ledger;
	private:
#ifdef DD_FINAL
		u32 ChangeCount = 0;
#else
		std::vector<Change> Changes;
#endif
		BitBoard _dirty;
	};

}

namespace ddahlkvist
{
	SUDOKULIB_PUBLIC SudokuContext buildContext(Board& b, Result& r);
}