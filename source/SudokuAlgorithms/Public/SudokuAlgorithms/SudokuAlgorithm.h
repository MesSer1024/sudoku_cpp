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

		struct NakedMatch {
			NakedMatch() = default;

			NakedMatch(u8* nodes, u8 count, u16 mask) {
				for (uint i = 0; i < 4; ++i)
					nodeIds[i] = i < count ? nodes[i] : 0;
				combinedMask = mask;
			}
			u16 nodeIds[4]{};
			u16 combinedMask{ 0 };

			bool operator==(const NakedMatch& other) const {
				bool nodeSame = (memcmp(nodeIds, other.nodeIds, 4) == 0);
				return nodeSame && combinedMask == other.combinedMask;

			}
		};

		struct HiddenMatch {
			u8 nodes[4]{};
			u16 candidateMask;
			u8 numNodes{};
		};

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
						} else {
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
				u8 nodeIds[9];
				const u8 numSolved = (p.Solved & dimension).fillSetBits(nodeIds);

				if (numSolved > 0 && numSolved < 9) {
					const u16 solvedValues = buildSolvedMask(p, nodeIds, numSolved);

					(dimension & p.Unsolved).foreachSetBit([&p, solvedValues](u32 nodeId) {
						Node& node = p.b.Nodes[nodeId];
						const bool haveCandidateToRemove = solvedValues & node.getCandidates();
						if (haveCandidateToRemove) {
							p.result.append(node, static_cast<u8>(nodeId));
							node.candidatesRemoveBySolvedMask(solvedValues);
						}
					});
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

			void whereBitCountGtDepth(NodeAction action) {
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
			// potentialNodes = foreach node in dimension : find all nodes with only [2..depth] candidates
			//		foreach_combination(potentialNodes) : depth = 3 {i,j,k}
			//			merge all candidates of the 3 nodes {i, j, k}, check if count of combined candidates <= depth [==Depth?] 3 nodes cannot share 2 candidates
			//				check if any other node in "dimension" has any of "combined_candidates", if so technique was successful and that candidate can be removed from neighbour
			// -----------------------------------------

			BitBoard allNakedNodes = BoardUtils::boardWhereCandidateCountEquals(p, depth);
			NodePermutationGenerator combo(p, allNakedNodes, depth);

			uint numMatches = 0;

			using NodeQueryAction = std::function<void(u8* nodeIds, u8 count)>;
			struct NodeQuery {
				BitBoard boardMask;
				const BitBoard* foreachCollection;
				u8 foreachSize;
				u8 whereBitCountGt;
				NodeQueryAction action;
			};
			
			NodeQuery query;
			query.boardMask = allNakedNodes;
			query.foreachCollection = p.AllDimensions.data();
			query.foreachSize = 27;
			query.whereBitCountGt = depth;
			query.action = [&numMatches, matches, &p, depth](u8* nodeIds, u8 count) {
				const u16 sharedCandidateMask = BoardUtils::mergeCandidateMasks(p, nodeIds, depth);
				if (countCandidates(sharedCandidateMask) <= depth) {
					matches[numMatches++] = NakedMatch(nodeIds, depth, sharedCandidateMask);
				}
			};
			//combo.foreachCombination(p.AllDimensions, depth)

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

		bool removeHiddenInternal(std::vector<HiddenMatch>& matches, const SudokuContext& p, u8 depth)
		{
			for (auto&& dimension : p.AllDimensions) {
				const BitBoard unsolvedDimension = dimension & p.Unsolved;
				const u8 numUnsolvedInDimension = unsolvedDimension.countSetBits();
				if (numUnsolvedInDimension <= depth) {
					// early out if tehcnique would not do anything
					continue;
				}

				auto transformWhereCountIsDepth = [](u8* outArray, u8* inArray, u8 depth) -> u8 {
					u8 numPotentials = 0;

					for (u8 i = 0; i < 9; ++i) {
						const u8 value = inArray[i];
						if (value >= 2 && value <= depth)
							outArray[numPotentials++] = i;
					}

					return numPotentials;
				};

				u8 numNodesWithCandidate[9];
				std::vector<u8> candidateIdsWithSufficientNodes(9);

				BoardUtils::populateCandidateCount(numNodesWithCandidate, p, unsolvedDimension);
				const u8 numPotentials = transformWhereCountIsDepth(candidateIdsWithSufficientNodes.data(), numNodesWithCandidate, depth);

				const bool techniqueCouldWork = numPotentials >= depth && numPotentials < numUnsolvedInDimension;
				if (techniqueCouldWork) {
					// foreach_candidate
					// merge bitboards for [a, b, c] (depth3) if they share same candidates, we have match

					auto begin = candidateIdsWithSufficientNodes.begin();
					for_each_combination(begin, begin + depth, begin + numPotentials, [&p, &unsolvedDimension, &matches](auto a, auto end) {
						const auto begin = a;
						const u8 depth = static_cast<u8>(end - a);
						BitBoard merged;
						while (a != end) {
							merged |= p.AllCandidates[*a];
							a++;
						}

						const BitBoard combo = merged & unsolvedDimension;
						const u8 numSharedNodes = combo.countSetBits();
						if (numSharedNodes >= 2 && numSharedNodes <= depth)
						{
							// validate that the nodes have more candidates than these
							u8 nodes[9];
							combo.fillSetBits(nodes);

							const u16 allNodeCandidates = BoardUtils::mergeCandidateMasks(p, nodes, numSharedNodes);
							const u16 numSharedCandidates = countCandidates(allNodeCandidates);
							if (numSharedCandidates > numSharedNodes) {

								u16 mask = 0;
								for (uint i = 0; i < numSharedNodes; ++i)
									mask |= 1u << (begin[i] + 1); // ids are zero based

								matches.push_back({});
								HiddenMatch& m = matches[matches.size() - 1];
								m.numNodes = numSharedNodes;
								m.candidateMask = mask;
								for(uint i=0; i < numSharedNodes; ++i)
									m.nodes[i] = nodes[i];
							}
						}

						return false;
					});
				}
			}

			return !matches.empty();
		}

		// hidden implies that a DIMENSION lacks CANDIDATE, and only a few nodes can have that value, if 2 candidate can only exist in 2 nodes, those values MUST be in those nodes
		bool removeHiddenPair(SudokuContext& p) {
			p.result.Technique = Techniques::HiddenPair;
			std::vector<HiddenMatch> matches;
			const u8 depth = 2;

			removeHiddenInternal(matches, p, depth);
			// in dimension, foreach candidate, count how many nodes have this candidate
			// if bitCount of candidates are shared by a few amount of nodes

			if (!matches.empty())
			{
				for (auto&& match : matches) {
					for (uint i = 0; i < match.numNodes; ++i) {
						const u8 nodeId = match.nodes[i];
						Node& node = p.b.Nodes[nodeId];
						p.result.append(node, nodeId);

						node.candidatesSet(match.candidateMask);
					}

				}

				return p.result.size() > 0;
			}
			return false;
		}

		bool removeHiddenTriplet(SudokuContext& p) {
			p.result.Technique = Techniques::HiddenTriplet;
			std::vector<HiddenMatch> matches;
			const u8 depth = 3;

			removeHiddenInternal(matches, p, depth);
			// in dimension, foreach candidate, count how many nodes have this candidate
			// if bitCount of candidates are shared by a few amount of nodes

			if (!matches.empty())
			{
				for (auto&& match : matches) {
					for (uint i = 0; i < match.numNodes; ++i) {
						const u8 nodeId = match.nodes[i];
						Node& node = p.b.Nodes[nodeId];
						p.result.append(node, nodeId);

						node.candidatesSet(match.candidateMask);
					}

				}

				return p.result.size() > 0;
			}
			return false;
		}
	}
}