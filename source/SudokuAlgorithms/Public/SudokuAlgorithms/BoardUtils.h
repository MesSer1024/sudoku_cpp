#pragma once

#include <algorithm>

#include <SudokuAlgorithms/SudokuTypes.h>

namespace dd
{
	// A pointer is a valid model of an iterator:
	inline bool NotSolvedPredicate(const Node& n)
	{
		return !n.isSolved();
	}

	inline bool SameRowPredicate(const Node& n, int row)
	{
		Board b;
		//getRow()
	}

	template<typename Func>
	uint fillNodesWithPredicate(Node* outNodes, const Board& b, Func f)
	{
		uint numFound = 0;

		auto begin = std::begin(b.Nodes);
		const auto end = std::end(b.Nodes);

		while (begin != end)
		{
			begin = std::find_if(begin, end, f);
			if (begin != end)
				outNodes[numFound++] = *begin;

		}
		return numFound;
	}
}