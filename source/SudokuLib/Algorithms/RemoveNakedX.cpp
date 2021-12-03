#pragma once

#include <SudokuLib/SudokuAlgorithm.h>
#include <SudokuLib/TechniqueMeta.h>
#include <BoardUtils.h>
#include "Combinations.h"

namespace ddahlkvist::techniques
{

	struct NakedMatch {
		NakedMatch() = default;

		NakedMatch(u8* nodes, u8 count, u16 with2nodes) {
			for (uint i = 0; i < 4; ++i)
				nodeIds[i] = i < count ? nodes[i] : 0;
			combinedMask = with2nodes;
		}
		u8 nodeIds[4]{};
		u16 combinedMask{ 0 };

		bool operator==(const NakedMatch& other) const {
			bool nodeSame = (memcmp(nodeIds, other.nodeIds, 4) == 0);
			return nodeSame && combinedMask == other.combinedMask;

		}
	};

	struct NodePermutationGenerator {
		const u8 MaxDepth = 4;

		explicit NodePermutationGenerator(const SudokuContext& p, BitBoard board, u8 depth)
			: _p(p)
			, _board(board)
			, _depth(depth)
		{
			assert(depth <= MaxDepth);
		}

		template<typename NodeAction>
		void whereBitCountGtDepth(NodeAction action) {
			if (_board.notEmpty() == false)
				return;

			for (auto&& dimension : _p.AllDimensions) {
				const BitBoard db = dimension & _board;
				if (db.countSetBits() < _depth)
					continue;

				// given that we can have 6 nodes and looking for pair, need to generate all combinations of pairs given {1,2,3,4,5,6} --> {1,2}, {1,3} ...
				u8 nodeIndexes[BoardSize];
				const u8 numSetBits = db.fillSetBits(nodeIndexes);

				auto begin = nodeIndexes;
				for_each_combination(begin, begin + _depth, begin + numSetBits, [&](auto a, auto b) -> bool {
					action(&*a);
					return false;
					});
			}
		}

	private:
		const SudokuContext& _p;
		BitBoard _board;
		u8 _depth;
	};

	uint gatherNakedPotentials(NakedMatch* matches, const SudokuContext& p, u8 depth) {
		// [candidates on node] naked implies that a NODE is limited to a known amount of candidates and hence, combining those naked nodes can remove candidate for other nodes
		// [candidates in dimension] hidden implies that a DIMENSION lacks CANDIDATE, and only a few nodes can have that value, if 2 candidate can only exist in 2 nodes, those values MUST be in those nodes

		// foreach node -> check amount of candidates (2/3) for pair, (2/3) for triplet, (2/3/4) for quad
		// -----------------------------------------
		// naive:
		// potentialNodes = foreach node in dimension : find all nodes with only [2..depth] candidates
		//		foreach_combination(potentialNodes) : depth = 3 {i,j,k}
		//			merge all candidates of the 3 nodes {i, j, k}, check if count of combined candidates <= depth [==Depth?] 3 nodes cannot share 2 candidates
		//				check if any other node in "dimension" has any of "combined_candidates", if so technique was successful and that candidate can be removed from neighbour
		// -----------------------------------------

		uint numMatches = 0;
		BitBoard allNakedNodes = BoardBits::nodesWithCandidateCountBetweenXY(p.AllCandidates, 2, depth);

		NodePermutationGenerator combo(p, allNakedNodes, depth);

		combo.whereBitCountGtDepth([&numMatches, matches, &p, depth](u8* nodeIds) {
			const u16 sharedCandidateMask = BoardUtils::mergeCandidateMasks(p, nodeIds, depth);
			if (countCandidates(sharedCandidateMask) <= depth) {
				matches[numMatches++] = NakedMatch(nodeIds, depth, sharedCandidateMask);
			}
			});
		return numMatches;
	}

	// [ All unsolved in dimension -> if 2 nodes only have 2 candidates and they are the same candidates, all other nodes in dimension can remove those candidates]
	bool removeNakedCandidatesInternal(SudokuContext& p, u8 depth) {
		NakedMatch matches[256];

		u32 numPotentials = gatherNakedPotentials(matches, p, depth);
		assert(numPotentials < 256);

		// see if any neighbours to the nodes located have any of marked candidates
		for (uint i = 0; i < numPotentials; ++i) {
			const NakedMatch& match = matches[i];

			const BitBoard sharedNeighbours = BoardBits::NeighboursUnion_ifAllNodesAreSameDimension(match.nodeIds, depth);
			const BitBoard nodesWithMaskedCandidates = BoardUtils::mergeCandidateBoards(p, match.combinedMask);
			const BitBoard affectedNodes = nodesWithMaskedCandidates & sharedNeighbours;

			if (affectedNodes.notEmpty()) {
				p.result.storePreModification(p.b.Nodes, affectedNodes);

				BoardUtils::removeCandidates(p, match.combinedMask, affectedNodes);
			}
		}

		return p.result.size() > 0;
	}

	bool removeNakedPair(SudokuContext& p) {
		p.result.Technique = Techniques::NakedPair;
		const u8 depth = 2;

		return removeNakedCandidatesInternal(p, depth);
	}

	bool removeNakedTriplet(SudokuContext& p) {
		p.result.Technique = Techniques::NakedTriplet;
		const u8 depth = 3;

		return removeNakedCandidatesInternal(p, depth);
	}

	bool removeNakedQuad(SudokuContext& p) {
		p.result.Technique = Techniques::NakedQuad;
		const u8 depth = 4;

		return removeNakedCandidatesInternal(p, depth);
	}
}