#pragma once

#include <algorithm>
#include <bitset>
#include <intrin.h>
#include <map>
#include <vector>

#include <iostream>
#include <vector>
#include <numeric>
#include <cstdint>
#include <cassert>

#include <SudokuAlgorithms/BoardUtils.h>
#include <SudokuAlgorithms/Module.h>
#include <SudokuAlgorithms/SudokuTypes.h>
#include <SudokuAlgorithms/Meta.h>

namespace dd
{

	namespace techniques
	{
		//---------------- Sudoku Solver -------------
		//http://www.thonky.com/sudoku/

		//Check for solved cells
		//Show Possibles	No

		//		x: Basic Candidates				removeNaiveCandidates() [All solved neighbours toMask -> remove as candidates on unsolved]
		//		0: Naked Single					removeNakedSingle()		[ All unsolved nodes -> If a node only has one candidate, it can be solved]
		//		1: Hidden Singles				removeHiddenSingle()	[ All unsolved neighbours -> if this is only occurance of candidate, it can be solved]
		//		2: Naked Pairs/Triples			removeNakedPair()		[ All unsolved in dimension -> if 2 nodes only have 2 candidates and share them, all other shared neighbours can remove those candidates]
		//										removeNakedTriplet()	[ All unsolved in dimension -> if 3 nodes only have 3 candidates and share them, all other shared neighbours can remove those candidates]
		//		3: Hidden Pairs/Triples	 
		//		4: Naked Quads	 
		//		5: Pointing Pairs	 
		//		6: Box/Line Reduction

		//Tough Strategies	
		//		7: X-Wing	 
		//		8: Simple Colouring	 
		//		9: Y-Wing	 
		//		10: Sword-Fish	 
		//		11: XYZ Wing	 
		//Diabolical Strategies	
		//		12: X-Cycles	 
		//		13: XY-Chain	 
		//		14: 3D Medusa	 
		//		15: Jelly-Fish	 
		//		16: Unique Rectangles	 
		//		17: Extended Unique Rect.	 
		//		18: Hidden Unique Rect's	 
		//		19: WXYZ Wing	 
		//		20: Aligned Pair Exclusion	 
		//Extreme Strategies	
		//		21: Grouped X-Cycles	 
		//		22: Empty Rectangles	 
		//		23: Finned X-Wing	 
		//		24: Finned Sword-Fish	 
		//		25: Altern. Inference Chains	 
		//		26: Sue-de-Coq	 
		//		27: Digit Forcing Chains	 
		//		28: Nishio Forcing Chains	 
		//		29: Cell Forcing Chains	 
		//		30: Unit Forcing Chains	 
		//		31: Almost Locked Sets	 
		//		32: Death Blossom	 
		//		33: Pattern Overlay Method	 
		//		34: Quad Forcing Chains	 
		//"Trial and Error"	
		//		35: Bowman's Bingo

		void fillAllUnsolvedWithAllCandidates(SudokuContext& p)
		{
			BitAction applyAllCandidatesAction = [&p](u32 bitIndex) {
				p.b.Nodes[bitIndex].candidatesSet(Candidates::All);
			};

			p.Unsolved.foreachSetBit(applyAllCandidatesAction);
		}

		bool removeNaiveCandidates(SudokuContext& p)
		{
			p.result.Technique = Techniques::NaiveCandidates;

			for (auto&& dimension : p.AllDimensions) {
				const u16 mask = toCandidateMask(BoardUtils::buildValueMaskFromSolvedNodes(p.b.Nodes, p.Solved & dimension));
				if (mask && mask != Candidates::All) {
					BitBoard modifiedNodes = BoardUtils::wouldRemoveCandidates(p.b.Nodes, p.Unsolved & dimension, mask);
					if (modifiedNodes.notEmpty())
					{
						p.result.storePreModification(p.b.Nodes, modifiedNodes);

						BoardUtils::removeCandidatesForNodes(p.b.Nodes, modifiedNodes, mask);
					}
				}
			}

			return p.result.size() > 0;
		}

		bool removeNakedSingle(SudokuContext& p)
		{
			p.result.Technique = Techniques::NakedSingle;

			BitBoard invalid = p.Solved;
			BitBoard touched;

			for (auto&& c : p.AllCandidates) {
				invalid |= (touched & c);
				touched |= c;
			}

			const BitBoard affectedNodes = touched & invalid.getInverted();
			if (affectedNodes.notEmpty()) {
				p.result.storePreModification(p.b.Nodes, affectedNodes);

				affectedNodes.foreachSetBit([&p](u32 bitIndex) {
					const u16 candidate = getOnlyCandidateFromMask(p.b.Nodes[bitIndex].getCandidates());
					p.b.Nodes[bitIndex].solve(candidate);
				});
			}

			return p.result.size() > 0;
		}

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

		struct HiddenMatch {
			u16 nodeIds[4]{};
			u16 candidateIds[4]{};

			u16 getCombinedCandidateValueMask(u32 depth) const {
				u16 mask = buildValueMaskFromCandidateIds(&candidateIds[0], depth);
				return mask;
			}
		};

		struct NakedMatch {
			NakedMatch() = default;

			NakedMatch(u8* nodes, u8 count, u16 mask) {
				for (uint i = 0; i < 4; ++i)
					nodeIds[i] = i < count ? nodes[i] : 0;
				combinedMask = mask;
			}
			u16 nodeIds[4]{};
			u16 combinedMask{0};

			bool operator==(const NakedMatch& other) const {
				bool nodeSame = (memcmp(nodeIds, other.nodeIds, 4) == 0);
				return nodeSame && combinedMask == other.combinedMask;
					
			}
		};

		class PermutationSolver {
			PermutationSolver() = delete;
			PermutationSolver(PermutationSolver&&) = delete;
			PermutationSolver(const PermutationSolver&) = delete;
		public:
			explicit PermutationSolver(u8 depth)
				: depth(depth)
			{
			}

			void run(u8 numPotentials, BitBoard* dimensionalNodes, u16* candidateIds) {
				numMatchesNew = 0; // reset

				potentialBoards = dimensionalNodes;
				potentialCandidateIds = candidateIds;

				std::vector<int> v(numPotentials);
				std::iota(v.begin(), v.end(), 0);

				for_each_combination(v.begin(), v.begin() + depth, v.end(), [&](auto a, auto b) -> bool {
					return InRange_LocateSharedBits_StoreEventualMatch(a, b);
				});
			}

			u32 fillMatches(HiddenMatch* matches) {
				for (uint i = 0; i < numMatchesNew; ++i) {
					matches[i] = foundMatches[i];
				}
				return numMatchesNew;
			}

		private:
			template <class It>
			bool InRange_LocateSharedBits_StoreEventualMatch(const It begin, const It end)
			{
				// foreach combination -> merge boards and count how many nodes
				It curr = begin;
				BitBoard merged;
				while (curr != end) {
					merged |= potentialBoards[*curr];
					curr++;
				}

				const u8 numSharedBits = merged.countSetBits();
				if (numSharedBits > depth) {
					return false;
				}

				u8 nodes[4];
				merged.fillSetBits(nodes);

				for (uint i = 0; i < depth; ++i) {
					foundMatches[numMatchesNew].nodeIds[i] = nodes[i];
					foundMatches[numMatchesNew].candidateIds[i] = potentialCandidateIds[begin[i]];
				}

				numMatchesNew++;
				return false;
			}

			BitBoard* potentialBoards{ nullptr };
			u16* potentialCandidateIds{ nullptr };
			HiddenMatch foundMatches[64];
			u8 depth{ 2 };
			u8 numMatchesNew{ 0 };
		};

		uint gatherHiddenCandidates() {
			// hidden implies that a DIMENSION lacks CANDIDATE, and only a few nodes can have that value, if 2 candidate can only exist in 2 nodes, those values MUST be in those nodes
			// naked implies that a NODE is limited to a known amount of candidates and hence, combining those naked nodes can remove candidate for other nodes
			assert(false); // #todo

			// naive:
			//generate bitboard related to  "candidateNodes & dimensionNodes":
			//count bits, and save this board if count >= 2 && <= depth

			//	foreach combination of saved board depth = 3{i, j, k} :
			//		merge the saved boards(i | j | k), count set bits
			//			if count == depth, this is a hidden triplet

			//	if any node in that triplet has any other candidate, that candidate should be removed

			return 0;
		}

		// this is actually hiddenPairTriplet...
		//uint gatherHiddenDimensionsThatHaveExactlyXCandidates(NakedPairMatch* matches, const SudokuContext& p, u8 depth) {
		//	uint numMatches = 0;
		//	PermutationSolver solver(depth);

		//	for (auto&& dimension : p.AllDimensions) {
		//		BoardBits::BitBoards9 potentialNakedMatchBoard;
		//		u16 potentialCandidateId[9];
		//		u8 numPotentials = 0;

		//		// check if any candidate is shared by exactly two nodes
		//		for (u8 candidateId = 0; candidateId < 9; ++candidateId) {
		//			const BitBoard cd = dimension & p.AllCandidates[candidateId];
		//			const u8 sharedBits = cd.size();
		//			const bool potentialNakedMatch = sharedBits > 1 && sharedBits <= depth;
		//			if (potentialNakedMatch) {
		//				potentialCandidateId[numPotentials] = candidateId;
		//				potentialNakedMatchBoard[numPotentials] = cd;

		//				numPotentials++;
		//			}
		//		}

		//		// see if two candidate-matches share the exact nodes
		//		if (numPotentials >= depth) {
		//			solver.run(numPotentials, potentialNakedMatchBoard.data(), potentialCandidateId);
		//			numMatches += solver.fillMatches(&matches[numMatches]);
		//		}
		//	}

		//	return numMatches;
		//}

		struct NodePermutationGenerator {
			const u8 MaxDepth = 4;

			explicit NodePermutationGenerator(const SudokuContext& p, BitBoard board, u8 depth)
				: _p(p)
				, _board(board)
				, _depth(depth)
			{
				assert(depth <= MaxDepth);
			}

			using NodeAction = std::function<void(u8* nodeIndices)>;

			void foreachDepthFilteredComboInDimension(NodeAction action) {
				if (_board.notEmpty() == false)
					return;

				for (auto&& dimension : _p.AllDimensions) {
					const BitBoard db = dimension & _board;
					if (db.countSetBits() < _depth)
						continue;

					// given that we can have 6 nodes and looking for pair, need to generate all combinations of pairs given {1,2,3,4,5,6} --> {1,2}, {1,3} ...
					std::vector<u8> nodeIndexes(BoardSize);
					const u8 numSetBits = db.fillSetBits(nodeIndexes.data());

					auto begin = nodeIndexes.begin();
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
			// potentialNodes = foreach node in dimension : find all nodes with [2..depth] candidates
			//		foreach_combination(potentialNodes) : depth = 3 {i,j,k}
			//			merge all candidates of the 3 nodes {i, j, k}, check if count of combined candidates <= depth [==Depth?] 3 nodes cannot share 2 candidates
			//				check if any other node in "dimension" has any of "combined_candidates", if so technique was successful and that candidate can be removed from neighbour
			// -----------------------------------------

			BitBoard allNakedNodes = BoardUtils::boardWithCandidateCount(p, depth);
			NodePermutationGenerator combo(p, allNakedNodes, depth);

			uint numMatches = 0;
			combo.foreachDepthFilteredComboInDimension([&numMatches, matches, &p, depth](u8* nodeIds) {
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

				const BitBoard sharedNeighbours = BoardBits::DistinctNeighboursClearSelf(match.nodeIds, depth);
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
	}
}