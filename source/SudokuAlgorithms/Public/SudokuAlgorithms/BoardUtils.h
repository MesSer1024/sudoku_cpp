#pragma once

#include <vector>

#include <SudokuAlgorithms/BoardUtils.h>
#include <SudokuAlgorithms/Module.h>
#include <SudokuAlgorithms/SudokuTypes.h>

namespace dd
{
	u32 buildValueMaskFromSolvedNodes(const Node* nodes, const BitBoard& affectedNodes)
	{
		u32 rowMask = 0U;

		affectedNodes.foreachSetBit([&rowMask, nodes](u32 bitIndex) {
			rowMask |= (1u << nodes[bitIndex].getValue());
		});

		return rowMask;
	}

	u16 toCandidateMask(u32 valueMask)
	{
		return (~valueMask) & Candidates::All;
	}

	u16 countCandidates(u16 candidates) {
		return __popcnt16(candidates);
	}

	u16 getOnlyCandidateFromMask(u16 candidates) {
		switch (candidates) {
		case Candidates::c1:
			return 1;
		case Candidates::c2:
			return 2;
		case Candidates::c3:
			return 3;
		case Candidates::c4:
			return 4;
		case Candidates::c5:
			return 5;
		case Candidates::c6:
			return 6;
		case Candidates::c7:
			return 7;
		case Candidates::c8:
			return 8;
		case Candidates::c9:
			return 9;
		}
		assert(false);
		return 0;
	}

	BoardBits::BitBoards9 buildCandidateBoards(Board& b) {
		BoardBits::BitBoards9 out;

		for (uint i = 0; i < BoardSize; ++i) {
			if (!b.Nodes[i].isSolved()) {
				u16 candidate = b.Nodes[i].getCandidates();
				for (u16 c = 1; c <= 9; ++c) {
					if (candidate & c) {
						out[c - 1].setBit(i);
					}
				}
			}
		}

		return out;
	}

	BitBoard wouldRemoveCandidates(Node* nodes, const BitBoard& affectedNodes, u32 possibleCandidateMask) {
		BitBoard modifiedNodes;
		affectedNodes.foreachSetBit([possibleCandidateMask, nodes, &modifiedNodes](u32 bitIndex) {
			u16 currCandidates = nodes[bitIndex].getCandidates();
			if ((possibleCandidateMask & currCandidates) != currCandidates)
				modifiedNodes.setBit(bitIndex);
		});
		return modifiedNodes;
	}

	void removeCandidatesForNodes(Node* nodes, const BitBoard& affectedNodes, u32 savedCandidates) {
		affectedNodes.foreachSetBit([savedCandidates, nodes](u32 bitIndex) {
			nodes[bitIndex].candidatesToKeep(static_cast<u16>(savedCandidates));
		});
	}
}