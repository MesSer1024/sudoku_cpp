#include <iostream>
#include <Core/Types.h>
#include <ExampleBoards.h>
#include <SudokuAlgorithms/SudokuTypes.h>
#include <SudokuAlgorithms/SudokuAlgorithm.h>
#include <assert.h>  

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
		u32 value = boards.size();
		assert(value > 0 && value < 7777777);

		Board b;
		assert(b.Nodes[0] == Node());

		printf("Received boards : numBoards=%u\n", value);
		std::string s = "..5.398...82.1...7.4.75.62..3.49.................23.8..91.82.6.5...6.93...894.1..";
		Board my = Board::fromString(s.c_str());
		assert(my.Nodes[0] == 0);
		assert(my.Nodes[1] == 0);
		assert(my.Nodes[2] == 5);
		assert(my != b);

		for (uint i = 0; i < BoardSize; ++i) {
			if (s[i] != '.') {
				u16 value = s[i] - '0';
				assert(my.Nodes[i] == value);
			}
		}
	}

	void validateHelpers() {
		std::string s = "..5.398...82.1...7.4.75.62..3.49.................23.8..91.82.6.5...6.93...894.1..";

			//	..5 .39 8..
			//	.82 .1. ..7
			//	.4. 75. 62.
				        
			//	.3. 49. ...
			//	... ... ...
			//	... .23 .8.
				        
			//	.91 .82 .6.
			//	5.. .6. 93.
			//	..8 94. 1..


		Board b = Board::fromString(s.c_str());
		for (uint foo = 0; foo < 9; ++foo)
		{
			Row row = getRow(b, foo);
			for (uint i = 0; i < 9; ++i)
			{
				assert(row.data[i] == b.Nodes[i+9*foo]);
			}
		}

		for (uint foo = 0; foo < 9; ++foo)
		{
			Column col = getColumn(b, foo);
			for (uint i = 0; i < 9; ++i)
			{
				assert(col.data[i] == b.Nodes[i * 9 + foo]);
			}
		}

		for (uint foo = 0; foo < 9; ++foo)
		{
			Cell cell = getCell(b, foo);
			assert(&cell.data != nullptr);
			u32 topLeft = topLeftFromCellId(foo);
			u32 expected = (foo % 3 * 3 + ((foo / 3) * 27));
			assert(topLeft == expected);
			assert(cell.data[0] == b.Nodes[expected]);
		}
	}
}

namespace dd
{
	void solveSimpleSudoku()
	{
		std::string s = "..5.398...82.1...7.4.75.62..3.49.................23.8..91.82.6.5...6.93...894.1..";
		Board board = Board::fromString(s.c_str());
		Result outcome;

		const bool modified = techniques::simple(board, outcome);
		assert(modified);
	}
}

int main()
{
	using namespace std;
	using namespace dd;

	validateCandidates();
	validateBuildBoardFromLayout();
	validateHelpers();

	solveSimpleSudoku();

	cin.get();
	return 0;
}
