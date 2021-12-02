#pragma once

#include <SudokuLib/SudokuAlgorithm.h>
#include <SudokuLib/TechniqueMeta.h>
#include <BoardUtils.h>

namespace ddahlkvist::techniques
{
	bool singleChain(SudokuContext& p) {
		p.result.Technique = Techniques::SingleChain;

		// If a candidate appears EXACTLY twice in a unit, then those two candidates are called a conjugate pair.
		// You can make use of Simple Coloring when a conjugate pair is "chained" with at least one other conjugate pair using the same candidate.

		// When you find a chain of conjugate pairs, you might be able to use simple coloring to remove candidates. 
		// Start with one conjugate pair. Since the pair has an either/or relationship, you can give each candidate a different color.

		// Keep doing this until you have colored all of the numbers in the singles chain.Because of the either/or relationship of conjugate pairs, 
		// you know that if one color is correct, then the candidates of the other color can all be eliminated.

		// rule: Color Appears Twice in a Unit
		// If candidates of the same color appear together in the same unit, then you know that that color is incorrect, 
		// because if that color were correct, then that candidate would have to be in two places in one unit, which is not allowed.
		// Therefore, all of the candidates of that color can be eliminated.

		// rule: Opposite colors in the same unit
		// The chain formed by the colored pairs might have several end points that don't connect with any other pairs. 
		// Sometimes, these chain ends appear together in the same unit, but they are of opposite colors, so they don't create a contradiction.
		// However, because of the either/or nature of conjugate pairs, you know that one of these end points must be the correct one.
		// That means that, for example, if you colored a chain of 4s, and if two 4s of different colors appear together in the same unit, 
		// AND there are some uncolored 4s in that unit as well, then you can eliminate all of those uncolored 4s.

		// rule: An Uncolored Candidate can See Two Oppositely-Colored Candidates
		// If an uncolored candidate is in the same unit as a colored candidate, it can be described as "seeing" that colored candidate.

		for (u8 i = 0; i < 9; ++i) {
			const u8 candidateId = i;
			const BitBoard candidates = p.AllCandidates[candidateId];
			const auto pairs = BoardBits::findConjugatePairsForCandidate(p, candidateId);
			BitBoard closed; // any handled node have looked at all neighbours, meaning we should not have to revisit it again

			auto buildValidOutgoingNodes = [&pairs](u8 a) -> BitBoard {
				BitBoard valid;
				for (const auto& pair : pairs) {
					const u8 p1 = pair.node1;
					const u8 p2 = pair.node2;
					if (a == p1)
						valid.setBit(p2);
					else if (a == p2)
						valid.setBit(p1);
				}

				return valid;
			};
			for (const auto& pair : pairs) {
				BitBoard open;
				BitBoard whiteMark;
				BitBoard blackMark;
				BitBoard blackPotential;
				BitBoard whitePotential;

				// check if it was already handled in previous iteration
				if (closed.test(pair.node1) || closed.test(pair.node2)) {
					assert(closed.test(pair.node1));
					assert(closed.test(pair.node2));
					continue;
				}

				// prepare first iteration
				open.setBit(pair.node1);
				open.setBit(pair.node2);
				whitePotential.setBit(pair.node1);
				blackPotential.setBit(pair.node2);



				while (open.notEmpty()) {

					const u8 node = open.firstOne();
					const bool wasClosed = closed.test(node);
					const bool useWhite = whitePotential.test(node);

					open.clearBit(node);
					closed.setBit(node);
					if (wasClosed) {

						continue; // I guess I should validate that we are here on same color as previously?
					}

					if (blackPotential.test(node) && whitePotential.test(node)) {
						whiteMark.setBit(node);
						blackMark.setBit(node);
						continue; // An Uncolored Candidate can See Two Oppositely-Colored Candidates
					}

					// do logic
					BitBoard& marked = useWhite ? whiteMark : blackMark;
					BitBoard& nextBoard = useWhite ? blackPotential : whitePotential;

					marked.setBit(node);

					const BitBoard neighbours = BoardBits::NeighboursForNode(node);
					const BitBoard alreadyHandled = (open | closed).invert();
					const BitBoard conjugatePairsForNode = buildValidOutgoingNodes(node);
					// need to validate that each "potential next node" is actually part of the links between "conjugate pairs"
					const BitBoard possibleNextSteps = neighbours & conjugatePairsForNode & alreadyHandled;
					if (possibleNextSteps.notEmpty()) {
						open |= possibleNextSteps;
						nextBoard |= possibleNextSteps;
					}
				}

				u8 closedNodes[81];
				const u8 numClosed = closed.fillSetBits(closedNodes);
				if (numClosed > 3) {
					// If a candidate is both marked as white and black (dunno if this rule is correct)
					const BitBoard overlapping = whiteMark & blackMark;
					if (overlapping.notEmpty()) {
						p.result.storePreModification(p.b.Nodes, overlapping);

						overlapping.foreachSetBit([&p, candidateId](u32 bit) {
							p.b.Nodes[bit].candidatesRemoveSingle(candidateId + 1);
							});

						return true; // dunno if this assumption is correct
					}

					// rule 2: if two nodes in the same dimension have the same color, that version of "color" is invalid
					for (auto&& dim : p.AllDimensions) {
						const BitBoard inDimensions[2] = { dim & whiteMark, dim & blackMark };
						for (uint a = 0; a < 2; ++a) {
							const BitBoard& inDimension = inDimensions[a];
							if (inDimension.countSetBits() > 1) {
								// all candidates of same colour can be removed (that version is invalid)
								p.result.storePreModification(p.b.Nodes, inDimension);

								inDimension.foreachSetBit([&p, candidateId](u32 bit) {
									p.b.Nodes[bit].candidatesRemoveSingle(candidateId + 1);
									});

								return true; // #error - should we really return here?

							}
						}
					}

					//rule 4|5: if a node can see both colours in any unit, it cannot have that candidate
					//I think this implementation is correct, nothing validates it...
					{
						BitBoard affectedNodes;
						const BitBoard unhandledNodes = (whiteMark | blackMark).invert() & candidates;

						unhandledNodes.foreachSetBit([&whiteMark, &blackMark, &affectedNodes, &candidates](u32 bit) {
							const BitBoard neighbours = BoardBits::NeighboursForNode(bit);
							if ((neighbours & whiteMark).notEmpty()) {
								if ((neighbours & blackMark).notEmpty()) {
									affectedNodes.setBit(bit);
								}
							}
							});

						if (affectedNodes.notEmpty()) {
							p.result.storePreModification(p.b.Nodes, affectedNodes);

							affectedNodes.foreachSetBit([&p, candidateId](u32 bit) {
								p.b.Nodes[bit].candidatesRemoveSingle(candidateId + 1);
								});

							return true; // #error - should we really return here?
						}
					} // rule 4|5
				}
			}
		}
		return p.result.size() > 0;
	}
}