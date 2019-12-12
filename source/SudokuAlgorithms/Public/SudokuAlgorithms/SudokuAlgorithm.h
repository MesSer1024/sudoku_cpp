#pragma once

#include <SudokuAlgorithms/Module.h>
#include <vector>
#include <map>

namespace dd
{
	struct Change
	{
		Node prev;
		Node next;
	};

	struct Result
	{
		void append(Node old, Node next)
		{
			Changes.push_back({ old, next });
		}

		Change fetch(uint idx)
		{
			return Changes[idx];
		}

		uint size() {
			return Changes.size();
		}

		bool operator()() {
			return Changes.size() > 0;
		}

	private:
		std::vector<Change> Changes;
	};

	namespace techniques
	{
		bool simple(const Board& b, Result& outResult)
		{
			
			return false;
		}
	}
}