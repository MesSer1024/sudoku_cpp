#pragma once

#include <SudokuLib/SudokuAlgorithm.h>
#include <SudokuLib/TechniqueMeta.h>
#include <BoardUtils.h>

namespace ddahlkvist::techniques
{
	bool removeYWing(SudokuContext& p) {
		p.result.Technique = Techniques::Y_Wing;

		// three different candidates (A, B, C)
		// they form a pattern where AB, BC, AC are on nodes that see eachother (think about a rectangle shape but we only care about 3 corners)
		// in that rectangle shape, the 4th corner is known to NEVER have candidate C since it must be in either AC or BC
		// hence the candidate can be removed from that position

		// this algorithm can also be applied to cover "block", which makes this technique very powerful...

		// one solution would be to check for all nodes that have exactly two candidates
		// then take all combination of those nodes and see if they can form a y-wing pattern
		struct YWingCombination {
			BitBoard affectedNodes;
			u8 candidateId;
		};

		YWingCombination ywings[100];
		u8 numYwings = 0;

		const BitBoard nodesWithExactly2Candidates = BoardBits::nodesWithCandidateCountBetweenXY(p.AllCandidates, 2, 2);

		BitBoard specificCandidates[9];
		u8 numSpecificCandidates[9];
		for (uint i = 0; i < 9; ++i) {
			specificCandidates[i] = nodesWithExactly2Candidates & p.AllCandidates[i];
			numSpecificCandidates[i] = specificCandidates[i].countSetBits();
		}

		const u8 numNodes = nodesWithExactly2Candidates.countSetBits();
		if (numNodes >= 3) {
			u8 nodes[81];
			assert(nodesWithExactly2Candidates.fillSetBits(nodes) == numNodes);

			for (uint i = 0; i < numNodes; ++i) {
				const u8 bit = nodes[i];
				u8 candidates[2];
				const u8 numCandidates = p.b.Nodes[bit].fillCandidateIds(candidates);
				assert(numCandidates == 2);

				const BitBoard neighboursWith2Candidates = BoardBits::NeighboursForNode(bit) & nodesWithExactly2Candidates;
				BitBoard neighboursWithC1 = neighboursWith2Candidates & p.AllCandidates[candidates[0]];
				BitBoard neighboursWithC2 = neighboursWith2Candidates & p.AllCandidates[candidates[1]];
				const BitBoard invalidNodes = neighboursWithC1 & neighboursWithC2;
				neighboursWithC1 ^= invalidNodes;
				neighboursWithC2 ^= invalidNodes;

				//////////////////////////////////
				u8 c1Nodes[9];
				u8 c2Nodes[9];
				const u8 numC1 = neighboursWithC1.fillSetBits(c1Nodes);
				const u8 numC2 = neighboursWithC2.fillSetBits(c2Nodes);

				u8 innerNodes[3];
				if (numC1 > 0 && numC2 > 0) {
					for (u8 a = 0; a < numC1; ++a) {
						for (u8 b = 0; b < numC2; ++b) {
							innerNodes[0] = bit;
							innerNodes[1] = c1Nodes[a];
							innerNodes[2] = c2Nodes[b];
							assert(innerNodes[0] != innerNodes[1]);
							assert(innerNodes[0] != innerNodes[2]);
							assert(innerNodes[1] != innerNodes[2]);

							const u16 candidateMask = BoardUtils::mergeCandidateMasks(p, innerNodes, 3);
							const u8 numInnerCandidates = countCandidates(candidateMask);
							if (numInnerCandidates == 3) {
								const u16 removed = candidateIdToMask(candidates[0]) | candidateIdToMask(candidates[1]);
								const u16 oneCandidate = candidateMask ^ removed;
								assert(countCandidates(oneCandidate) == 1);
								const u8 searchedCandidateId = static_cast<u8>(getOnlyCandidateFromMask(oneCandidate) - 1);

								const BitBoard allSeenNodes = BoardBits::NeighboursIntersection(&innerNodes[1], 2); // take the 2 other nodes (except the one I was iterating over
								const BitBoard seenWithCandidateAndPotential = allSeenNodes & p.AllCandidates[searchedCandidateId];
								if (seenWithCandidateAndPotential.notEmpty()) {
									YWingCombination& ywing = ywings[numYwings++];
									ywing.affectedNodes = seenWithCandidateAndPotential;
									ywing.candidateId = searchedCandidateId;
								}
							}
						}
					}
				}
			}
		}

		for (uint i = 0; i < numYwings; ++i) {
			YWingCombination& ywing = ywings[i];
			p.result.storePreModification(p.b.Nodes, ywing.affectedNodes);

			ywing.affectedNodes.foreachSetBit([&p, &ywing](u32 bit) {
				p.b.Nodes[bit].candidatesRemoveSingle(ywing.candidateId + 1);
				});
		}
		return p.result.size() > 0;
	}
}