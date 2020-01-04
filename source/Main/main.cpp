#include <assert.h>  
#include <functional>
#include <iostream>

#include <ExampleBoards.h>
#include <Validations.h>
#include <Core/Types.h>
#include <SudokuAlgorithms/SudokuTypes.h>
#include <SudokuAlgorithms/SudokuAlgorithm.h>

namespace dd
{
	bool invokeTechniques(Board& b, Result& result)
	{
		//if (!techniques::soloCandidate(b, result))
		//	;
		return false;
	}
}

void runValidations()
{
	using namespace dd;

	validateCandidates();
	validateBuildBoardFromLayout();
	validateBitHelpers();

	validateCandidateAddAndSimpleRemoval();
	validateTechniques();
}

int main()
{
	runValidations();

	using namespace std;
	using namespace dd;


	
	Board board = Board::fromString(ExampleBoardRaw.c_str());
	Result outcome;

	int i = 0;
	while (true && i < 2000)
	{
		if (invokeTechniques(board, outcome))
		{
			// something was improved
		}
		i++;
	}

	cout << (i >= 2000 ? "Unsolved" : "solved");

	cin.get();
	return 0;
}
