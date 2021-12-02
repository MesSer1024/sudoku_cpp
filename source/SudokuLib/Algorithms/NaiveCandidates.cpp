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

		for (auto&& dimension : p.AllDimensions) {
			u8 unsolvedNodes[9];
			const BitBoard unsolved = p.Unsolved & dimension;
			const u8 numUnsolved = unsolved.fillSetBits(unsolvedNodes);
			const u16 mergedMask = BoardUtils::mergeCandidateMasks(p, unsolvedNodes, numUnsolved);
			if (countCandidates(mergedMask) == numUnsolved)
				continue;

			const BitBoard solvedDimension = p.Solved & dimension;
			const u16 solvedValues = buildSolvedMask(p, solvedDimension);
			for (uint i = 0; i < numUnsolved; ++i) {
				const u8 nodeId = unsolvedNodes[i];
				Node& node = p.b.Nodes[nodeId];
				const bool haveCandidateToRemove = solvedValues & node.getCandidates();
				if (haveCandidateToRemove) {
					p.result.append(node, static_cast<u8>(nodeId));
					node.candidatesRemoveBySolvedMask(solvedValues);
				}

			}
		}

		return p.result.size() > 0;
	}
}