#pragma once
#include <assert.h>  
#include <iostream>
#include <functional>

#include <ExampleBoards.h>
#include <Core/Types.h>
#include <SudokuAlgorithms/SudokuTypes.h>
#include <SudokuAlgorithms/SudokuAlgorithm.h>

namespace dd
{
	void validateCandidates() {
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

		assert(hasC4 == false);
		assert(exceptC4 == false);
		assert(exceptC4c8 == true);

		printf("Validated candidates!\n");
	}

	void validateBuildBoardFromLayout() {
		BoardCollection boards = dd::GetRawBoards();
		u32 value = static_cast<u32>(boards.size());
		assert(value > 0 && value < 7777777);

		Board b;
		assert(b.Nodes[0] == Node());

		printf("Received boards : numBoards=%u\n", value);
		Board my = Board::fromString(ExampleBoardRaw.c_str());
		assert(my.Nodes[0] == 0);
		assert(my.Nodes[1] == 0);
		assert(my.Nodes[2] == 5);
		assert(my != b);

		for (uint i = 0; i < BoardSize; ++i) {
			if (ExampleBoardRaw[i] != '.') {
				u16 value = ExampleBoardRaw[i] - '0';
				assert(my.Nodes[i] == value);
			}
		}
	}

	void validateStaticUtilBitBoards()
	{
		{
			BoardBits::SudokuBitBoard bitBoard;
			for (uint i = 0; i < 128; ++i)
				assert(bitBoard.test(i) == false);
		}
		{
			// test tresholds 0 and 63
			BoardBits::SudokuBitBoard bitBoard;
			bitBoard.setBit(0);
			bitBoard.setBit(63);
			assert(bitBoard.test(0));
			assert(bitBoard.test(63));
		}
		{
			// test tresholds 64
			BoardBits::SudokuBitBoard bitBoard;
			bitBoard.setBit(64);
			assert(bitBoard.test(64));
		}
		{
			// test tresholds 65 66
			BoardBits::SudokuBitBoard bitBoard;
			bitBoard.setBit(65);
			bitBoard.setBit(66);
			assert(bitBoard.test(65));
			assert(bitBoard.test(66));
		}
		{
			// test modify tresholds 63..66
			BoardBits::SudokuBitBoard bitBoard;
			bitBoard.modifyBit(63, true);
			bitBoard.modifyBit(64, true);
			bitBoard.modifyBit(65, true);
			bitBoard.modifyBit(66, true);
			assert(bitBoard.test(63));
			assert(bitBoard.test(64));
			assert(bitBoard.test(65));
			assert(bitBoard.test(66));
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

			assert(bitBoard.test(63) == false);
			assert(bitBoard.test(64) == false);
			assert(bitBoard.test(65) == false);
			assert(bitBoard.test(66) == false);
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

			assert(bitBoard == ExpectedBitBoard);
		}

		{
			BoardBits::SudokuBitBoard bitBoard(BoardBits::SudokuBitBoard::All{});
			for (uint i = 0; i < BoardSize; ++i)
				assert(bitBoard.test(i));
			assert(bitBoard.test(BoardSize) == false);
		}

		{
			const uint RowId = 3;
			const uint ColumnId = 3;
			const uint blockId = 3;

			BoardBits::SudokuBitBoard row = BoardBits::BitRow(RowId);
			BoardBits::SudokuBitBoard column = BoardBits::BitColumn(ColumnId);
			BoardBits::SudokuBitBoard block = BoardBits::BitBlock(blockId);

			assert(row.countSetBits() == 9);
			assert(column.countSetBits() == 9);
			assert(block.countSetBits() == 9);
			
			for (uint i = 0; i < 9; ++i)
			{
				assert(row.test(RowId * 9 + i));
				assert(column.test(i * 9 + ColumnId));
			}

			assert(block.test(27));
			assert(block.test(28));
			assert(block.test(29));
			assert(block.test(36));
			assert(block.test(37));
			assert(block.test(38));
			assert(block.test(45));
			assert(block.test(46));
			assert(block.test(47));
		}

		{
			BoardBits::BitBoards3 neighbourBitBoards = BoardBits::NeighboursForNode(0);
			BoardBits::SudokuBitBoard ExpectedRow = BoardBits::BitRow(0);
			BoardBits::SudokuBitBoard ExpectedColumn = BoardBits::BitColumn(0);
			BoardBits::SudokuBitBoard ExpectedBlock = BoardBits::BitBlock(0);
			assert(neighbourBitBoards[0] == ExpectedRow);
			assert(neighbourBitBoards[1] == ExpectedColumn);
			assert(neighbourBitBoards[2] == ExpectedBlock);
		}

		{
			BoardBits::BitBoards3 neighbourBitBoards = BoardBits::NeighboursForNode(43); // { 4, 7, 5 }
			BoardBits::SudokuBitBoard ExpectedRow = BoardBits::BitRow(4);
			BoardBits::SudokuBitBoard ExpectedColumn = BoardBits::BitColumn(7);
			BoardBits::SudokuBitBoard ExpectedBlock = BoardBits::BitBlock(5);
			assert(neighbourBitBoards[0] == ExpectedRow);
			assert(neighbourBitBoards[1] == ExpectedColumn);
			assert(neighbourBitBoards[2] == ExpectedBlock);
		}

		{
			for (uint i = 0; i < BoardSize; ++i)
			{
				BitBoard neighbours = BoardBits::NeighboursForNodeCombined(i);
				assert(neighbours.countSetBits() == 20);
			}
		}

		{
			const u32 NodeId = 2;
			BitBoard neighbours = BoardBits::NeighboursForNodeCombined(NodeId);
			BoardBits::SudokuBitBoard ExpectedRow = BoardBits::BitRow(BoardBits::RowForNodeId(NodeId));
			BoardBits::SudokuBitBoard ExpectedColumn = BoardBits::BitColumn(BoardBits::ColumnForNodeId(NodeId));
			BoardBits::SudokuBitBoard ExpectedBlock = BoardBits::BitBlock(BoardBits::BlockForNodeId(NodeId));

			ExpectedRow.clearBit(NodeId);
			ExpectedColumn.clearBit(NodeId);
			ExpectedBlock.clearBit(NodeId);

			assert((neighbours & ExpectedRow).countSetBits() == 8u);
			assert((neighbours & ExpectedColumn).countSetBits() == 8u);
			assert((neighbours & ExpectedBlock).countSetBits() == 8u);
			assert((neighbours & ExpectedRow & ExpectedBlock).countSetBits() == 2u);
			assert((neighbours & ExpectedRow & ExpectedColumn).countSetBits() == 0u);
		}
	}

	void validateBoardAndBitBoardTransformations()
	{
		Board b = Board::fromString(ExampleBoardRaw.c_str());

		{
			BoardBits::SudokuBitBoard bitBoardSolved = BoardBits::bitsSolved(b);
			BoardBits::SudokuBitBoard bitBoardUnsolved = BoardBits::bitsUnsolved(b);
			for (uint i = 0; i < BoardSize; ++i)
			{
				if (b.Nodes[i].isSolved()) {
					assert(bitBoardSolved.test(i));
					assert(bitBoardUnsolved.test(i) == false);
				} else {
					assert(bitBoardUnsolved.test(i));
					assert(bitBoardSolved.test(i) == false);
				}
			}

			BoardBits::SudokuBitBoard flippedAndMasked = bitBoardSolved.getInverted();
			assert(flippedAndMasked == bitBoardUnsolved);
		}

		{
			const u16 c1Mask = Candidates::c1;
			const u16 ModifiedNode = 33;
			assert(!b.Nodes[ModifiedNode].isSolved());
			b.Nodes[ModifiedNode].candidatesSet(c1Mask);

			const BitBoard bitsWithC1 = BoardBits::buildCandidateBoards(b)[0];

			assert(bitsWithC1.countSetBits() == 1);
			assert(bitsWithC1.test(ModifiedNode));

			b = Board::fromString(ExampleBoardRaw.c_str()); // reset
		}
	}

	void validateBitHelpers() {
		validateStaticUtilBitBoards();
		validateBoardAndBitBoardTransformations();
	}
}


namespace dd
{
	void validateRemoveNaiveCandidates()
	{
		Result ignoredResult;

		{
			Board board;
			board.Nodes[0].solve(1);

			SudokuContext ctx{ board, ignoredResult, BoardBits::bitsSolved(board), BoardBits::bitsUnsolved(board),{}, BoardBits::AllDimensions() };
			techniques::fillUnsolvedWithNonNaiveCandidates(ctx);

			const BitBoard unsolved = BoardBits::bitsUnsolved(board);
			BitBoard unsolvedNeighbours = BoardBits::NeighboursForNodeCombined(0) & unsolved;
			const u16 ExpectedCandidates = Candidates::All & (~Candidates::c1);
			const u32 numUnsolvedNeighbours = unsolvedNeighbours.countSetBits();

			assert(numUnsolvedNeighbours == 20);

			unsolvedNeighbours.foreachSetBit([&board, ExpectedCandidates](u32 bitIndex) {
				assert(board.Nodes[bitIndex].getCandidates() == ExpectedCandidates);
			});
		}
		{
			Board board;
			board.Nodes[0].solve(1);
			board.Nodes[1].solve(2);
			board.Nodes[9].solve(2);

			SudokuContext ctx{ board, ignoredResult, BoardBits::bitsSolved(board), BoardBits::bitsUnsolved(board),{}, BoardBits::AllDimensions() };
			techniques::fillUnsolvedWithNonNaiveCandidates(ctx);

			const BitBoard unsolved = BoardBits::bitsUnsolved(board);
			const BitBoard unsolvedNeighbours = BoardBits::NeighboursForNodeCombined(0) & unsolved;
			const u16 ExpectedCandidates = Candidates::All & (~(Candidates::c1 | Candidates::c2));
			const u32 numUnsolvedNeighbours = unsolvedNeighbours.countSetBits();

			assert(numUnsolvedNeighbours == 18);
			unsolvedNeighbours.foreachSetBit([&board, ExpectedCandidates](u32 bitIndex) {
				assert(board.Nodes[bitIndex].getCandidates() == ExpectedCandidates);
			});
		}
	}

	void validateCandidateAddAndSimpleRemoval()
	{
		validateRemoveNaiveCandidates();
	}

	void validateSoloCandidateTechnique()
	{
		Board board;
		Result outcome;
		board.Nodes[64].candidatesSet(1 << 5);

		SudokuContext context{
			board,
			outcome,
			BoardBits::bitsSolved(board),
			BoardBits::bitsUnsolved(board),
			BoardBits::buildCandidateBoards(board),
			BoardBits::AllDimensions()
		};

		const bool modified = techniques::removeNakedSingle(context);
		assert(modified);
		assert(outcome.size() == 1);
		assert(outcome.fetch(0).index == 64);
	}

	void validateTechniques()
	{
		validateSoloCandidateTechnique();
	}
}