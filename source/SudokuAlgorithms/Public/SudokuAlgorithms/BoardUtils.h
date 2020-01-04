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