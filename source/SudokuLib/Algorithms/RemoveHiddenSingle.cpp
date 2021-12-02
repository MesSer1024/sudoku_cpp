#pragma once

#include <SudokuLib/SudokuAlgorithm.h>
#include <SudokuLib/TechniqueMeta.h>
#include <BoardUtils.h>

namespace ddahlkvist::techniques
{
	// [ All unsolved in dimension -> if candidate only exists in one node in dimension, it can be solved]
	bool removeHiddenSingle(SudokuContext& p) {
		p.result.Technique = Techniques::HiddenSingle;

		BitBoard unhandledNodes(BitBoard::All{});
		u8 targetValue[BoardSize];
		u8 affectedNodes[BoardSize];
		u8 numAffectedNodes = 0;

		{
			for (uint c = 0; c < 9; ++c) {
				for (uint i = 0; i < 27; ++i) {
					// for all candidates , all possible directions, find unsolved with that candidate that we have not already fixed
					const BitBoard maskedNodesWithCandidate = p.AllDimensions[i] & p.AllCandidates[c];
					if (maskedNodesWithCandidate.countSetBits() == 1) {
						const u32 bitPos = maskedNodesWithCandidate.firstOne();

						if (unhandledNodes.test(bitPos))
						{
							unhandledNodes.clearBit(bitPos);

							affectedNodes[numAffectedNodes] = static_cast<u8>(bitPos);
							targetValue[numAffectedNodes] = static_cast<u8>(c + 1);

							numAffectedNodes++;
						}
					}
				}
			}
		}

		if (numAffectedNodes > 0) {
			for (uint i = 0; i < numAffectedNodes; ++i) {
				const u8 nodeId = affectedNodes[i];
				p.result.append(p.b.Nodes[nodeId], nodeId);
			}
			for (uint i = 0; i < numAffectedNodes; ++i) {
				const u8 nodeId = affectedNodes[i];
				const u8 value = targetValue[i];
				p.b.Nodes[nodeId].solve(value);
			}
		}

		return numAffectedNodes > 0;
	}

}