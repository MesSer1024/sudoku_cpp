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
		node.setCandidatesFromMask(Candidates::All);
		node.removeCandidate(4);
		node.removeCandidate(8);
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
		BoardCollection boards = dd::GetBoards();
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
			const u64 ExpectedLowerMask = 1ULL << 63 | 1ULL << 0;
			const u64 ExpectedUpperMask = 1ULL << (64 % 64) | 1ULL << (65 % 64);

			bitBoard.setBit(0);
			bitBoard.setBit(63);

			bitBoard.setBit(64);
			bitBoard.setBit(65);

			assert(bitBoard.test(0));
			assert(bitBoard.test(63));
			assert(bitBoard.test(64));

			assert(bitBoard.bits[0] == ExpectedLowerMask);
			assert(bitBoard.bits[1] == ExpectedUpperMask);
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
			const uint CellId = 3;

			BoardBits::SudokuBitBoard row = BoardBits::BitRow(RowId);
			BoardBits::SudokuBitBoard column = BoardBits::BitColumn(ColumnId);
			BoardBits::SudokuBitBoard cell = BoardBits::BitCell(CellId);

			assert(row.count() == 9);
			assert(column.count() == 9);
			assert(cell.count() == 9);
			
			for (uint i = 0; i < 9; ++i)
			{
				assert(row.test(RowId * 9 + i));
				assert(column.test(i * 9 + ColumnId));
			}

			assert(cell.test(27));
			assert(cell.test(28));
			assert(cell.test(29));
			assert(cell.test(36));
			assert(cell.test(37));
			assert(cell.test(38));
			assert(cell.test(45));
			assert(cell.test(46));
			assert(cell.test(47));
		}

		{
			BoardBits::BitBoards3 neighbourBitBoards = BoardBits::NeighboursForNode(0);
			BoardBits::SudokuBitBoard ExpectedRow = BoardBits::BitRow(0);
			BoardBits::SudokuBitBoard ExpectedColumn = BoardBits::BitColumn(0);
			BoardBits::SudokuBitBoard ExpectedCell = BoardBits::BitCell(0);
			assert(neighbourBitBoards[0] == ExpectedRow);
			assert(neighbourBitBoards[1] == ExpectedColumn);
			assert(neighbourBitBoards[2] == ExpectedCell);
		}

		{
			BoardBits::BitBoards3 neighbourBitBoards = BoardBits::NeighboursForNode(43); // { 4, 7, 5 }
			BoardBits::SudokuBitBoard ExpectedRow = BoardBits::BitRow(4);
			BoardBits::SudokuBitBoard ExpectedColumn = BoardBits::BitColumn(7);
			BoardBits::SudokuBitBoard ExpectedCell = BoardBits::BitCell(5);
			assert(neighbourBitBoards[0] == ExpectedRow);
			assert(neighbourBitBoards[1] == ExpectedColumn);
			assert(neighbourBitBoards[2] == ExpectedCell);
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
			b.Nodes[ModifiedNode].setCandidatesFromMask(c1Mask);

			BoardBits::SetBitForNodePredicate hasCandidate1 = [c1Mask](const Node& n) { return n.getCandidates() == c1Mask; };
			BoardBits::SudokuBitBoard bitsWithC1 = BoardBits::bitsPredicate(b, hasCandidate1);
			assert(bitsWithC1.count() == 1);
			assert(bitsWithC1.test(ModifiedNode));

			b = Board::fromString(ExampleBoardRaw.c_str()); // reset
		}
	}

	void validateBitHelpers() {
		validateStaticUtilBitBoards();
		validateBoardAndBitBoardTransformations();
	}

	//void validateHelpers() {
		//Board b = Board::fromString(ExampleBoardRaw.c_str());
		//for (uint foo = 0; foo < 9; ++foo)
		//{
		//	Row row = getRow(b, foo);
		//	for (uint i = 0; i < 9; ++i)
		//	{
		//		assert(row.data[i] == b.Nodes[i + 9 * foo]);
		//	}
		//}

		//for (uint foo = 0; foo < 9; ++foo)
		//{
		//	Column col = getColumn(b, foo);
		//	for (uint i = 0; i < 9; ++i)
		//	{
		//		assert(col.data[i] == b.Nodes[i * 9 + foo]);
		//	}
		//}

		//for (uint foo = 0; foo < 9; ++foo)
		//{
		//	Cell cell = getCell(b, foo);
		//	assert(&cell.data != nullptr);
		//	u32 topLeft = topLeftFromCellId(foo);
		//	u32 expected = (foo % 3 * 3 + ((foo / 3) * 27));
		//	assert(topLeft == expected);
		//	assert(cell.data[0] == b.Nodes[expected]);
		//}
	//}
}


namespace dd
{
	void validateAddCandidates()
	{
		Board board = Board::fromString(ExampleBoardRaw.c_str());
		const s64 preNumCandidates = std::count_if(std::begin(board.Nodes), std::end(board.Nodes), [](const Node& node) { return node.getCandidates() == Candidates::All; });
		const s64 preSolvedCandidates = std::count_if(std::begin(board.Nodes), std::end(board.Nodes), [](const Node& node) { return node.isSolved(); });

		techniques::fillAllUnsolvedWithAllCandidates(board);
		
		const s64 postNumCandidates = std::count_if(std::begin(board.Nodes), std::end(board.Nodes), [](const Node& node) { return node.getCandidates() == Candidates::All; });
		const s64 postSolvedCandidates = std::count_if(std::begin(board.Nodes), std::end(board.Nodes), [](const Node& node) { return node.isSolved(); });

		assert(preNumCandidates == 0);
		assert(preSolvedCandidates == postSolvedCandidates);
		assert(postSolvedCandidates + postNumCandidates == BoardSize);
	}

	void validateSoloCandidateTechnique()
	{
		Board board = Board::fromString(ExampleBoardRaw.c_str());
		Result outcome;

		board.Nodes[64].setCandidatesFromMask(1 << 5);
		const bool modified = techniques::soloCandidate(board, outcome);
		assert(modified);
		assert(outcome.size() == 1);
		assert(outcome.fetch(0).index == 64);
	}

	void validateTechniques()
	{
		validateSoloCandidateTechnique();
	}
}