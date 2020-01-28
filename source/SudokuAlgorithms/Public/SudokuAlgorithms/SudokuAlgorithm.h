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
#include <SudokuAlgorithms/TechniqueMeta.h>

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

			BitBoard allNakedNodes = BoardUtils::boardWhereCandidateCountInRange(p, depth);
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

				const BitBoard sharedNeighbours = BoardBits::SharedNeighborsClearSelf(match.nodeIds, depth);
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

				BoardUtils::countTimesEachCandidateOccur(numNodesWithCandidate, p, unsolvedDimension);
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

		bool removeNakedQuad(SudokuContext& p) {
			p.result.Technique = Techniques::NakedQuad;
			const u8 depth = 4;

			return removeNakedCandidatesInternal(p, depth);
		}

		bool removeHiddenQuad(SudokuContext& p) {
			p.result.Technique = Techniques::HiddenQuad;
			std::vector<HiddenMatch> matches;
			const u8 depth = 4;

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

		bool pointingPair(SudokuContext& p) {
			p.result.Technique = Techniques::PointingPair;

			// foreach block check candidates -> if candidate only exists 2-3 times in block
				// if affected nodes are in same row/column
					// this is a pointing pair
					// all other nodes in that row/column can remove that candidate

			struct Match {
				u8 numNodes{};
				u8 nodes[4]{};
				uint candidateId;
			};

			Match matches[81];
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
							Match& match = matches[numMatches];
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
					const Match& match = matches[i];
					const BitBoard neighbours = BoardBits::SharedNeighborsClearSelf(match.nodes, match.numNodes);
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

		bool boxLineReduction(SudokuContext& p) {
			p.result.Technique = Techniques::BoxLineReduction;

			// this technique is related to looking at a column/row and making sure that a value MUST exist in that row.
			// if we know that the value must reside in ONE ROW and it is in the same block, all other nodes within that block can remove candidate since it must appear on that row/col 

			// search for candidates contained in a row or column where they within that row/column are all in the same block (2-3)
			// (candidates & row) -> "have same block" || (candidates & col) -> "have same block"
			//		IF -> (candidates & blockId_from_previous) != (candidates & row) | (candidates & col)
			//			(candidates & blockId_from_previous) ^ (candidates & row) | (candidates & col)

			for (uint c = 0; c < 9; ++c) {
				const BitBoard& candidates = p.AllCandidates[c];
				for (uint i = 0; i < 9; ++i) {
					const BitBoard boards[2] = { candidates & BoardBits::BitRow(i), candidates & BoardBits::BitColumn(i) };

					for (auto&& dimension : boards) {
						if (dimension.countSetBits() >= 2) {
							u8 blockId;
							if (BoardBits::sharesBlock(blockId, dimension)) {
								const BitBoard inBlock = candidates & BoardBits::BitBlock(blockId);
								const BitBoard Diff = dimension ^ inBlock;
								if (Diff.notEmpty()) {
									p.result.storePreModification(p.b.Nodes, Diff);

									const u16 candidate = static_cast<u16>(c + 1);
									Diff.foreachSetBit([&p, candidate](u32 bit) {

										p.b.Nodes[bit].candidatesRemoveSingle(candidate);
									});
								}
							}
						}
					}
				}
			}

			return p.result.size() > 0;
		}

		struct XwingCombination {
			BitBoard AffectedNodes;
			BitBoard RectangleNodes;
			u8 candidateId;
		};

		template<typename Output, class Collection, typename WhereClause>
		u8 SelectWhere(Output* out, const Collection& ref, WhereClause func)
		{
			using iterator_type = Collection::const_iterator;
			using collection_type = CandidateBoard;

			u8 numHits = 0;
			iterator_type begin = std::begin(ref);
			const iterator_type end = std::end(ref);

			while (begin != end) {
				const collection_type& board = *begin;
				if (func(board))
					out[numHits++] = board;
				begin++;
			}

			return numHits;
		}

		struct RectangleQueryResult {
			bool success;
			u8 trialId1;
			u8 trialId2;
			BitBoard rectangle;

			//operator bool() { return success; }
		};

		RectangleQueryResult canFormRectangle(const BitBoard& dimension1, const CandidateBoards9& trialsOtherDimension) {
			assert(dimension1.countSetBits() == 4u);
			const u8 end = static_cast<u8>(trialsOtherDimension.size());
			for (u8 i = 0; i < end; ++i) {
				if (trialsOtherDimension[i].numNodes < 2)
					continue;
				for (u8 j = i + 1; j < end; ++j) {
					if (trialsOtherDimension[j].numNodes < 2)
						continue;

					const BitBoard mergedTrials = trialsOtherDimension[i].board | trialsOtherDimension[j].board;
					const BitBoard merged = dimension1 & mergedTrials;
					const u8 numMerged = merged.countSetBits();
					if (numMerged == 4u) {
						return RectangleQueryResult{ true, i, j, merged };
					}
				}
			}
			return RectangleQueryResult{};
		}

		bool xWing(SudokuContext& p) {
			p.result.Technique = Techniques::X_Wing;

			//When there are only two possible cells for a value in each of two different rows,
			//	and these candidates lie also in the same columns, (forms rectangle)
			//	then all other candidates for this value in the columns can be eliminated.
			//		or cells -> rows

			auto findRectangles = [](RectangleQueryResult* out, const CandidateBoard* boards, u8 numBoards, const CandidateBoards9& lookup) {
				u8 numRectangles = 0;

				for (uint i = 0; i < numBoards; ++i) {
					for (uint j = i + 1; j < numBoards; ++j) {
						const BitBoard mergedBoard = boards[i].board | boards[j].board;
						out[numRectangles] = canFormRectangle(mergedBoard, lookup);

						if (out[numRectangles].success) {
							numRectangles++;
						}
					}
				}

				return numRectangles;
			};

			auto buildCombos = [](XwingCombination* out, const RectangleQueryResult* rectangles, u8 queryCount, const CandidateBoards9& lookup) {
				u8 numXwings = 0;

				for (uint i = 0; i < queryCount; ++i) {
					const RectangleQueryResult& query = rectangles[i];
					XwingCombination& xwing = out[numXwings];

					xwing.AffectedNodes = (lookup[query.trialId1].board | lookup[query.trialId2].board);
					if (xwing.AffectedNodes.countSetBits() < 4)
						continue; // don't use items with too few affected nodes

					numXwings++;
					xwing.RectangleNodes = query.rectangle;
					xwing.candidateId = lookup[query.trialId1].candidateId;
				}

				return numXwings;
			};

			XwingCombination xwings[100];
			uint numXwings = 0;

			// check for strict candiates over all rows and columns
			for (u8 c = 0; c < 9; ++c) {
				CandidateBoards9 rows = BoardBits::buildAllRowsForCandidate(p, c);
				CandidateBoards9 cols = BoardBits::buildAllColumnsForCandidate(p, c);
				CandidateBoard subset[9];
				RectangleQueryResult rectangles[9];

				// foreach row_combo_2_rows --> check if they share same columns [can form rectangle with 2 other columns]
				// if rectangle can be formed, then all other nodes with that candidate IN_COLUMNS can be removed (except the nodes used to form rectangle)

				// check strict match of rows --> columns
				if (const u8 numSubsetBoards = SelectWhere(subset, rows, [](const CandidateBoard& b) {return b.numNodes == 2u; }))
					if (const u8 numRectangles = findRectangles(rectangles, subset, numSubsetBoards, cols))
						numXwings += buildCombos(&xwings[numXwings], rectangles, numRectangles, cols);

				// check strict match of columns --> rows
				if (const u8 numSubsetBoards = SelectWhere(subset, cols, [](const CandidateBoard& b) {return b.numNodes == 2u; }))
					if (const u8 numRectangles = findRectangles(rectangles, subset, numSubsetBoards, rows))
						numXwings += buildCombos(&xwings[numXwings], rectangles, numRectangles, rows);
			}

			// APPLY CHANGES
			// we had a candidate that were forced into a "rectangle position" by for instance only occuring twice in two rows, where the rows also shared columns
			// any node seeing the rectangle (where the node is not part of rectangle) we can remove the candidate from that node...
			for (uint i = 0; i < numXwings; ++i) {
				const XwingCombination& xwing = xwings[i];
				const BitBoard nodes = xwing.AffectedNodes ^ xwing.RectangleNodes;
				assert(xwing.AffectedNodes.countSetBits() - nodes.countSetBits() == 4u);

				if (nodes.countSetBits() > 0) {
					p.result.storePreModification(p.b.Nodes, nodes);

					nodes.foreachSetBit([&p, &xwing](u32 bit) {
						p.b.Nodes[bit].candidatesRemoveSingle(xwing.candidateId + 1);
					});
				}
			}

			return p.result.size() > 0;
		}

		bool yWing(SudokuContext& p) {
			p.result.Technique = Techniques::Y_Wing;

			// three different candidates (A, B, C)
			// they form a pattern where AB, BC, AC are on nodes that see eachother (think about a rectangle shape but we only care about 3 corners)
			// in that rectangle shape, the 4th corner is known to NEVER have candidate C since it must be in either AC or BC
			// hence the candidate can be removed from that position

			// this algorithm can also be applied to cover "block", which makes this technique very powerful...

			// one solution would be to check for all nodes that have exactly two candidates
			// then take all combination of those nodes and see if they can form a y-wing pattern


			const BitBoard nodesWithExactly2Candidates = BoardUtils::boardWhereCandidateCountInRange(p, 2, 2);
			BitBoard specificCandidates[9];
			u8 numSpecificCandidates[9];
			for (uint i = 0; i < 9; ++i) {
				specificCandidates[i] = nodesWithExactly2Candidates & p.AllCandidates[i];
				numSpecificCandidates[i] = specificCandidates[i].countSetBits();
			}

			struct MyCandidates {
				u8 c1;
				u8 c2;
			};
			auto getMy2Candidates = [](BitBoard* specificCandidates, u8 nodeId) -> MyCandidates {
				u8 c[2];
				u8 numCandidats = 0;
				for (u8 i = 0; i < 9; ++i)
					if (specificCandidates[i].test(nodeId))
						c[numCandidats++] = i;

				return { c[0], c[1] };
			};


			struct MyPair {
				u8 nodeId;
				u8 candidateId;
				u8 hingeId;
			};

			auto findPinnedPairs = [](SudokuContext& p, const u8 rootNodeId, const BitBoard& possibilities, u8 whereC1, u8 whereC2) {
				std::vector<MyPair> pairs;
				assert(whereC1 != whereC2);
				assert(whereC1 < 9);
				assert(whereC2 < 9);

				BitBoard c1Board = p.AllCandidates[whereC1];
				BitBoard c2Board = p.AllCandidates[whereC2];
				c1Board.clearBit(rootNodeId);
				c2Board.clearBit(rootNodeId);

				const uint rowId = BoardUtils::RowForNodeId(rootNodeId);
				const uint colId = BoardUtils::ColumnForNodeId(rootNodeId);
				const uint blockId = BoardUtils::BlockForNodeId(rootNodeId);

				BitBoard c1Boards[3] = {
					c1Board & BoardBits::BitRow(rowId),
					c1Board & BoardBits::BitColumn(colId), 
					c1Board & BoardBits::BitBlock(blockId), 
				};

				BitBoard c2Boards[3] = {
					c2Board & BoardBits::BitRow(rowId),
					c2Board & BoardBits::BitColumn(colId),
					c2Board & BoardBits::BitBlock(blockId)
				};

				for (uint i = 0; i < 3; ++i) {
					if (c1Boards[i].countSetBits() == 1) {
						const u8 pairNodeId = c1Boards[i].firstOne();
						const bool isC1 = c1Board.test(pairNodeId);
						const bool isC2 = c2Board.test(pairNodeId);
						if (isC1 != isC2) {
							const u8 candidateId = whereC1;
							pairs.push_back(MyPair{ pairNodeId, candidateId, rootNodeId });
						}
					}
					if (c2Boards[i].countSetBits() == 1) {
						const u8 pairNodeId = c2Boards[i].firstOne();
						const bool isC1 = c1Board.test(pairNodeId);
						const bool isC2 = c2Board.test(pairNodeId);
						if (isC1 != isC2) {
							const u8 candidateId = whereC2;
							pairs.push_back(MyPair{ pairNodeId, candidateId, rootNodeId });
						}
					}
				}

				return pairs;
			};

			const u8 numNodes = nodesWithExactly2Candidates.countSetBits();
			if (numNodes >= 3) {
				u8 nodes[81];
				const u8 numNodes = nodesWithExactly2Candidates.fillSetBits(nodes);
				for (uint i = 0; i < numNodes; ++i) {
					const u8 bit = nodes[i];
					auto candidates = getMy2Candidates(specificCandidates, bit);
					auto pairs = findPinnedPairs(p, bit, nodesWithExactly2Candidates, candidates.c1, candidates.c2);
					if (pairs.size() >= 2) {
						// pairs might be something similar to: 
						//	+ [0]{ nodeId = 0 '\0' candidateId = 3 '\x3' }	dd::techniques::yWing::__l2::MyPair
						//	+ [1]{ nodeId = 16 '\x10' candidateId = 3 '\x3' }	dd::techniques::yWing::__l2::MyPair
						//	+ [2]{ nodeId = 16 '\x10' candidateId = 3 '\x3' }	dd::techniques::yWing::__l2::MyPair
						//	+ [3]{ nodeId = 4 '\x4' candidateId = 8 '\b' }	dd::techniques::yWing::__l2::MyPair
						//	+ [4]{ nodeId = 34 '\"' candidateId = 8 '\b' }	dd::techniques::yWing::__l2::MyPair
						//	+ [5]{ nodeId = 26 '\x1a' candidateId = 8 '\b' }	dd::techniques::yWing::__l2::MyPair
						
						// here comes the fun part: if two PAIR with different candidates from the above list, share a node
						// if that node happen to share a candidate shared by pair... it can be removed
						// so, we have found out that our node shares candidate with other nodes 49 -> 45 & 59,
						// now we need to figure out if node seen by "pair" share another candidate in our scenario if (5) is shared by common neighbour from our pair

						int apa = 0;
					}
				}
			}

			return p.result.size() > 0;
		}

		using TechniqueFunction = std::function<bool(SudokuContext& p)>;
		std::vector<TechniqueFunction> allTechniques() {
			std::vector<TechniqueFunction> out = {
				removeNaiveCandidates,
				removeNakedSingle, removeHiddenSingle,
				removeNakedPair, removeNakedTriplet, 
				removeHiddenPair, removeHiddenTriplet, 
				removeNakedQuad, removeHiddenQuad,
				pointingPair,
				boxLineReduction,
				xWing, yWing
			};
			return out;
		}
	} // techniques
} // dd