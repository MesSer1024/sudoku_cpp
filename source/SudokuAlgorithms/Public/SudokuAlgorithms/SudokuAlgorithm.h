#pragma once

#include <SudokuAlgorithms/Module.h>
#include <vector>
#include <map>
#include <bitset>
#include <intrin.h>

namespace dd
{
	struct Change
	{
		Node prev;
		Node next;
		u8 index;
	};

	struct Result
	{
		void append(u8 id, Node old, Node next)
		{
			Changes.push_back({ old, next, id });
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
		bool soloCandidate(const Board& b, Result& outResult)
		{
			uint i = 0;
			for (Node n : b.Nodes)
			{
				std::bitset<9> candidates(n.getCandidates());
				if (candidates.count() == 1)
				{
					unsigned long bitIndex;
					_BitScanForward(&bitIndex, candidates.to_ulong());
					Node old = n;
					Node next = n;
					next.solve(bitIndex);
					outResult.append(i, old, next);
					return true;
				}
				i++;
			}
			return false;
		}
	}
}