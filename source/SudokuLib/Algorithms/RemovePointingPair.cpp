#pragma once

#include <SudokuLib/SudokuAlgorithm.h>
#include <SudokuLib/TechniqueMeta.h>
#include <BoardUtils.h>

namespace ddahlkvist::techniques
{
		bool removePointingPair(SudokuContext& p) {
			p.result.Technique = Techniques::PointingPair;

			// foreach block check candidates -> if candidate only exists 2-3 times in block
				// if affected nodes are in same row/column
					// this is a pointing pair
					// all other nodes in that row/column can remove that candidate

			struct PointingPairMatch {
				u8 numNodes{};
				u8 nodes[4]{};
				uint candidateId;
			};

			PointingPairMatch matches[81];
			uint numMatches = 0;

			const uint firstBlock = 18;
			for (uint i = 0; i < 9; ++i) {
				auto&& block = p.AllDimensions[firstBlock + i];
				uint c = 0;
				for (auto&& candidateBoard : p.AllCandidates) {
					const BitBoard filteredNodes = block & candidateBoard;
					u8 nodes[9];
					const u8 numNodes = filteredNodes.fillSetBits(nodes);
					if (numNodes > 0 && numNodes <= 3) {
						const bool sameRow = BoardUtils::sharesRow(nodes, numNodes);
						const bool sameCol = BoardUtils::sharesColumn(nodes, numNodes);

						if (sameRow || sameCol) {
							PointingPairMatch& match = matches[numMatches];
							uint j = 0;
							while (j < 4) {
								match.nodes[j] = nodes[j];
								j++;
							}
							match.numNodes = numNodes;
							match.candidateId = c;
							numMatches++;
						}
					}
					c++;
				}
			}

			if (numMatches > 0) {
				for (uint i = 0; i < numMatches; ++i) {
					const PointingPairMatch& match = matches[i];
					const BitBoard neighbours = BoardBits::NeighboursUnion_ifAllNodesAreSameDimension(match.nodes, match.numNodes);
					const BitBoard neighboursWithCandidate = p.AllCandidates[match.candidateId] & neighbours;
					if (neighboursWithCandidate.notEmpty()) {
						p.result.storePreModification(p.b.Nodes, neighboursWithCandidate);
						neighboursWithCandidate.foreachSetBit([&p, &match](u32 bit) {

							const u16 candidate = static_cast<u16>(match.candidateId + 1);
							p.b.Nodes[bit].candidatesRemoveSingle(candidate);
						});
					}
				}
			}

			return p.result.size() > 0;
		}

}