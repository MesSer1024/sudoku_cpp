#pragma once

#include <SudokuLib/SudokuAlgorithm.h>
#include <SudokuLib/TechniqueMeta.h>
#include <BoardUtils.h>

namespace ddahlkvist::techniques
{
	void fillUnsolvedWithNonNaiveCandidates(SudokuContext& p)
	{
		p.result.Technique = Techniques::None;
		BitBoard touchedNodes;

		for (auto&& dimension : p.AllDimensions) {
			u8 nodeIds[9];
			const u8 numSolved = (p.Solved & dimension).fillSetBits(nodeIds);

			if (numSolved < 9) {
				const u16 solvedValues = buildSolvedMask(p, nodeIds, numSolved);
				const u16 candidateMask = toCandidateMask(solvedValues);

				(dimension & p.Unsolved).foreachSetBit([&p, &touchedNodes, solvedValues, candidateMask](u32 nodeId) {
					Node& node = p.b.Nodes[nodeId];
					if (touchedNodes.test(nodeId) == false) {
						touchedNodes.setBit(nodeId);
						node.candidatesSet(candidateMask);
					}
					else {
						node.candidatesRemoveBySolvedMask(solvedValues);
					}
					});
			}
		}
		p.result.storePreModification(p.b.Nodes, touchedNodes);
	}

	bool removeNaiveCandidates(SudokuContext& p)
	{
		p.result.Technique = Techniques::NaiveCandidates;

		// This algorithm will remove candidates from nodes because the value has been solved inside row/col/block [dimension]
		// Meaning: if we have solved the value 4 inside block, then we remove 4 as a candidate from all other nodes inside that block

		// other attempt
		// it feels like the current solution is doing a lot of if/else and taking small scale decisions by building and populating arrays, feels like it SHOULD be possible to solve the problem taking wider sweeps...
		// it feels like we could utilize BitBoard for "solved" nodes and then compare that to "dimension + candidates"
		// baseline for performance: 5ms, 3ms spent inside removeNaiveCandidates

		// ~[32-40]ms usually ~35ms
		for (u16 value = 0; value < 9; ++value)
		{
			const BitBoard& nodesWhereValueIsKnown = p.SolvedValues[value];
			if (!nodesWhereValueIsKnown.notEmpty())
				continue;

			for (auto&& dimension : p.AllDimensions)
			{
				const BitBoard solvedInDimension = nodesWhereValueIsKnown & dimension;
				if (solvedInDimension.notEmpty())
				{
					const BitBoard badCandidates = p.AllCandidates[value] & dimension;
					badCandidates.foreachSetBit([&](auto idx) {
						Node& node = p.b.Nodes[idx];
						p.result.append(node, idx);
						const u16 candidateId = value + 1;
						node.candidatesRemoveSingle(candidateId);
					});
				}
			}
		}

		return p.result.size() > 0;
	}
}