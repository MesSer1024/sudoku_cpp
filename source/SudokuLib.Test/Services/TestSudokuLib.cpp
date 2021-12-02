#include <gtest/gtest.h>

#include <Core/Types.h>
#include <assert.h>  
#include <iostream>
#include <functional>

#include <Core/Types.h>
#include <SudokuLib/SudokuTypes.h>
#include <SudokuLib/SudokuAlgorithm.h>
#include <BoardUtils.h>

namespace ddahlkvist
{
	class SudokuLibFixture : public testing::Test {
	public:
		static constexpr char const* ExampleBoardRaw = "..5.398...82.1...7.4.75.62..3.49.................23.8..91.82.6.5...6.93...894.1..";

		SudokuLibFixture()
			: _maxValue(17)
		{}

	protected:
		void SetUp() override {

		}
		void TearDown() override {
		}

		u32 _maxValue;
	};

	TEST_F(SudokuLibFixture, isInvoked) {
		EXPECT_EQ(_maxValue, 17u);
		Board board;
		Result result;
		auto data = buildContext(board, result);
	}

	TEST_F(SudokuLibFixture, validateCandidates)
	{
		Node node;
		node.candidatesSet(Candidates::All);
		node.candidatesRemoveSingle(4);
		node.candidatesRemoveSingle(8);
		const u16 c4Mask = Candidates::c4;
		const u16 candidates = node.getCandidates();
		const u16 exceptC4Mask = (~c4Mask) & Candidates::All;
		const u16 exceptC4C8Mask = ~(c4Mask | Candidates::c8) & Candidates::All;
		bool hasC4 = candidates & c4Mask;
		bool exceptC4 = candidates == exceptC4Mask;
		bool exceptC4c8 = candidates == exceptC4C8Mask;

		EXPECT_TRUE(hasC4 == false);
		EXPECT_TRUE(exceptC4 == false);
		EXPECT_TRUE(exceptC4c8 == true);
	}

	TEST_F(SudokuLibFixture, validateBuildBoardFromLayout)
	{
		Board b;
		EXPECT_TRUE(b.Nodes[0] == Node());

		Board my = Board::fromString(ExampleBoardRaw);
		EXPECT_TRUE(my.Nodes[0] == 0);
		EXPECT_TRUE(my.Nodes[1] == 0);
		EXPECT_TRUE(my.Nodes[2] == 5);
		EXPECT_TRUE(my != b);

		for (uint i = 0; i < BoardSize; ++i) {
			if (ExampleBoardRaw[i] != '.') {
				u16 innerValue = ExampleBoardRaw[i] - '0';
				EXPECT_TRUE(my.Nodes[i] == innerValue);
			}
		}
	}

	TEST_F(SudokuLibFixture, validateStaticUtilBitBoards)
	{
		{
			BoardBits::SudokuBitBoard bitBoard;
			for (uint i = 0; i < 128; ++i)
				EXPECT_TRUE(bitBoard.test(i) == false);
		}
		{
			// test tresholds 0 and 63
			BoardBits::SudokuBitBoard bitBoard;
			bitBoard.setBit(0);
			bitBoard.setBit(63);
			EXPECT_TRUE(bitBoard.test(0));
			EXPECT_TRUE(bitBoard.test(63));
		}
		{
			// test tresholds 64
			BoardBits::SudokuBitBoard bitBoard;
			bitBoard.setBit(64);
			EXPECT_TRUE(bitBoard.test(64));
		}
		{
			// test tresholds 65 66
			BoardBits::SudokuBitBoard bitBoard;
			bitBoard.setBit(65);
			bitBoard.setBit(66);
			EXPECT_TRUE(bitBoard.test(65));
			EXPECT_TRUE(bitBoard.test(66));
		}
		{
			// test modify tresholds 63..66
			BoardBits::SudokuBitBoard bitBoard;
			bitBoard.modifyBit(63, true);
			bitBoard.modifyBit(64, true);
			bitBoard.modifyBit(65, true);
			bitBoard.modifyBit(66, true);
			EXPECT_TRUE(bitBoard.test(63));
			EXPECT_TRUE(bitBoard.test(64));
			EXPECT_TRUE(bitBoard.test(65));
			EXPECT_TRUE(bitBoard.test(66));
		}
		{
			// test modify tresholds 63..66
			BoardBits::SudokuBitBoard bitBoard;
			bitBoard.setBit(63);
			bitBoard.setBit(64);
			bitBoard.setBit(65);
			bitBoard.setBit(66);

			bitBoard.modifyBit(63, false);
			bitBoard.modifyBit(64, false);
			bitBoard.modifyBit(65, false);
			bitBoard.modifyBit(66, false);

			EXPECT_TRUE(bitBoard.test(63) == false);
			EXPECT_TRUE(bitBoard.test(64) == false);
			EXPECT_TRUE(bitBoard.test(65) == false);
			EXPECT_TRUE(bitBoard.test(66) == false);
		}
		{
			BoardBits::SudokuBitBoard bitBoard;

			bitBoard.setBit(0);
			bitBoard.setBit(63);

			bitBoard.setBit(64);
			bitBoard.setBit(65);

			const u64 ExpectedLowerMask = 1ULL << 63 | 1ULL << 0;
			const u64 ExpectedUpperMask = 1ULL << (64 % 64) | 1ULL << (65 % 64);
			const BitBoard ExpectedBitBoard(ExpectedLowerMask, ExpectedUpperMask);

			EXPECT_TRUE(bitBoard == ExpectedBitBoard);
		}

		{
			BoardBits::SudokuBitBoard bitBoard(BoardBits::SudokuBitBoard::All{});
			for (uint i = 0; i < BoardSize; ++i)
				EXPECT_TRUE(bitBoard.test(i));
			EXPECT_TRUE(bitBoard.test(BoardSize) == false);
		}

		{
			const uint RowId = 3;
			const uint ColumnId = 3;
			const uint blockId = 3;

			BoardBits::SudokuBitBoard row = BoardBits::BitRow(RowId);
			BoardBits::SudokuBitBoard column = BoardBits::BitColumn(ColumnId);
			BoardBits::SudokuBitBoard block = BoardBits::BitBlock(blockId);

			EXPECT_TRUE(row.countSetBits() == 9);
			EXPECT_TRUE(column.countSetBits() == 9);
			EXPECT_TRUE(block.countSetBits() == 9);

			for (uint i = 0; i < 9; ++i)
			{
				EXPECT_TRUE(row.test(RowId * 9 + i));
				EXPECT_TRUE(column.test(i * 9 + ColumnId));
			}

			EXPECT_TRUE(block.test(27));
			EXPECT_TRUE(block.test(28));
			EXPECT_TRUE(block.test(29));
			EXPECT_TRUE(block.test(36));
			EXPECT_TRUE(block.test(37));
			EXPECT_TRUE(block.test(38));
			EXPECT_TRUE(block.test(45));
			EXPECT_TRUE(block.test(46));
			EXPECT_TRUE(block.test(47));
		}
		{
			for (uint i = 0; i < BoardSize; ++i)
			{
				BitBoard neighbours = BoardBits::NeighboursForNode(i);
				EXPECT_TRUE(neighbours.countSetBits() == 20);
			}
		}

		{
			const u32 NodeId = 2;
			BitBoard neighbours = BoardBits::NeighboursForNode(NodeId);
			BoardBits::SudokuBitBoard ExpectedRow = BoardBits::BitRow(BoardUtils::RowForNodeId(NodeId));
			BoardBits::SudokuBitBoard ExpectedColumn = BoardBits::BitColumn(BoardUtils::ColumnForNodeId(NodeId));
			BoardBits::SudokuBitBoard ExpectedBlock = BoardBits::BitBlock(BoardUtils::BlockForNodeId(NodeId));

			ExpectedRow.clearBit(NodeId);
			ExpectedColumn.clearBit(NodeId);
			ExpectedBlock.clearBit(NodeId);

			EXPECT_TRUE((neighbours & ExpectedRow).countSetBits() == 8u);
			EXPECT_TRUE((neighbours & ExpectedColumn).countSetBits() == 8u);
			EXPECT_TRUE((neighbours & ExpectedBlock).countSetBits() == 8u);
			EXPECT_TRUE((neighbours & ExpectedRow & ExpectedBlock).countSetBits() == 2u);
			EXPECT_TRUE((neighbours & ExpectedRow & ExpectedColumn).countSetBits() == 0u);
		}
	}

	TEST_F(SudokuLibFixture, validateBoardAndBitBoardTransformations)
	{
		Board b = Board::fromString(ExampleBoardRaw);

		{
			BoardBits::BitBoards9 valueSolved;
			BoardBits::SudokuBitBoard bitBoardSolved;
			BoardBits::SudokuBitBoard bitBoardUnsolved;

			BoardBits::fillBitsSolved(valueSolved, bitBoardSolved, b);
			bitBoardUnsolved = bitBoardSolved.invert();
			for (uint i = 0; i < BoardSize; ++i)
			{
				if (b.Nodes[i].isSolved()) {
					EXPECT_TRUE(bitBoardSolved.test(i));
					EXPECT_TRUE(bitBoardUnsolved.test(i) == false);
				}
				else {
					EXPECT_TRUE(bitBoardUnsolved.test(i));
					EXPECT_TRUE(bitBoardSolved.test(i) == false);
				}
			}

			BoardBits::SudokuBitBoard flippedAndMasked = bitBoardSolved.invert();
			EXPECT_TRUE(flippedAndMasked == bitBoardUnsolved);
		}

		{

			const u16 c1Mask = Candidates::c1;
			const u16 ModifiedNode = 33;
			EXPECT_TRUE(!b.Nodes[ModifiedNode].isSolved());
			b.Nodes[ModifiedNode].candidatesSet(c1Mask);

			Result ignored;
			SudokuContext ctx = buildContext(b, ignored);

			const BitBoard bitsWithC1 = ctx.AllCandidates[0];

			EXPECT_TRUE(bitsWithC1.countSetBits() == 1);
			EXPECT_TRUE(bitsWithC1.test(ModifiedNode));

			b = Board::fromString(ExampleBoardRaw); // reset
		}
	}

	TEST_F(SudokuLibFixture, validateRemoveNaiveCandidates)
	{
		Result ignoredResult;

		{
			Board board;
			board.Nodes[0].solve(1);

			SudokuContext ctx = buildContext(board, ignoredResult);
			techniques::fillUnsolvedWithNonNaiveCandidates(ctx);

			const BitBoard unsolved = ctx.Unsolved;
			BitBoard unsolvedNeighbours = BoardBits::NeighboursForNode(0) & unsolved;
			const u16 ExpectedCandidates = Candidates::All & (~Candidates::c1);
			const u32 numUnsolvedNeighbours = unsolvedNeighbours.countSetBits();

			EXPECT_TRUE(numUnsolvedNeighbours == 20);

			unsolvedNeighbours.foreachSetBit([&board, ExpectedCandidates](u32 bitIndex) {
				EXPECT_TRUE(board.Nodes[bitIndex].getCandidates() == ExpectedCandidates);
				});
		}
		{
			Board board;
			board.Nodes[0].solve(1);
			board.Nodes[1].solve(2);
			board.Nodes[9].solve(2);

			SudokuContext ctx = buildContext(board, ignoredResult);
			techniques::fillUnsolvedWithNonNaiveCandidates(ctx);

			const BitBoard unsolved = ctx.Unsolved;
			const BitBoard unsolvedNeighbours = BoardBits::NeighboursForNode(0) & unsolved;
			const u16 ExpectedCandidates = Candidates::All & (~(Candidates::c1 | Candidates::c2));
			const u32 numUnsolvedNeighbours = unsolvedNeighbours.countSetBits();

			EXPECT_TRUE(numUnsolvedNeighbours == 18);
			unsolvedNeighbours.foreachSetBit([&board, ExpectedCandidates](u32 bitIndex) {
				EXPECT_TRUE(board.Nodes[bitIndex].getCandidates() == ExpectedCandidates);
				});
		}
	}

	TEST_F(SudokuLibFixture, validateSoloCandidateTechnique)
	{
		Board board;
		Result outcome;
		board.Nodes[64].candidatesSet(1 << 5);

		SudokuContext context = buildContext(board, outcome);

		const bool modified = techniques::removeNakedSingle(context);
		EXPECT_TRUE(modified);
		EXPECT_TRUE(outcome.size() == 1);
		EXPECT_TRUE(outcome.fetch(0).index == 64);
	}

}
