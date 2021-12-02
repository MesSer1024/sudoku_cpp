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
		// This algorithm will remove candidates from nodes because the value has been solved inside row/col/block [dimension]
		// Meaning: if we have solved the value 4 inside block, then we remove 4 as a candidate from all other nodes inside that block

		// other attempt
		// it feels like the current solution is doing a lot of if/else and taking small scale decisions by building and populating arrays, feels like it SHOULD be possible to solve the problem taking wider sweeps...
		// it feels like we could utilize BitBoard for "solved" nodes and then compare that to "dimension + candidates"
		// baseline for performance: 5ms, 3ms spent inside removeNaiveCandidates

		//BitBoard solved_1; // assume that we have a bitboard containing nodes that are known to hold the [value 1]





		p.result.Technique = Techniques::NaiveCandidates;

		//for (auto&& dimension : p.AllDimensions) {
		//	u8 unsolvedNodes[9];
		//	const BitBoard unsolved = p.Unsolved & dimension;
		//	const u8 numUnsolved = unsolved.fillSetBits(unsolvedNodes);
		//	const u16 mergedMask = BoardUtils::mergeCandidateMasks(p, unsolvedNodes, numUnsolved);
		//	if (countCandidates(mergedMask) == numUnsolved)
		//		continue; // what does this early out do?

		//	const BitBoard solvedDimension = p.Solved & dimension;
		//	const u16 solvedValues = buildSolvedMask(p, solvedDimension); // all solved values in dimension
		//	for (uint i = 0; i < numUnsolved; ++i) {
		//		const u8 nodeId = unsolvedNodes[i];
		//		Node& node = p.b.Nodes[nodeId];
		//		const bool haveCandidateToRemove = solvedValues & node.getCandidates(); // compare the mask of "solved" with "mask of candidates" foreach node
		//		if (haveCandidateToRemove) {
		//			p.result.append(node, static_cast<u8>(nodeId));
		//			node.candidatesRemoveBySolvedMask(solvedValues);
		//		}

		//	}
		//}

		for (auto&& dimension : p.AllDimensions) {
			u8 unsolvedNodes[9];
			const BitBoard unsolved = p.Unsolved & dimension;
			const u8 numUnsolved = unsolved.fillSetBits(unsolvedNodes);
			const u16 mergedMask = BoardUtils::mergeCandidateMasks(p, unsolvedNodes, numUnsolved);
			if (countCandidates(mergedMask) == numUnsolved)
				continue; // what does this early out do?

			const BitBoard solvedDimension = p.Solved & dimension;
			const u16 solvedValues = buildSolvedMask(p, solvedDimension); // all solved values in dimension
			for (uint i = 0; i < numUnsolved; ++i) {
				const u8 nodeId = unsolvedNodes[i];
				Node& node = p.b.Nodes[nodeId];
				const bool haveCandidateToRemove = solvedValues & node.getCandidates(); // compare the mask of "solved" with "mask of candidates" foreach node
				if (haveCandidateToRemove) {
					p.result.append(node, static_cast<u8>(nodeId));
					node.candidatesRemoveBySolvedMask(solvedValues);
				}

			}
		}

		return p.result.size() > 0;
	}
}