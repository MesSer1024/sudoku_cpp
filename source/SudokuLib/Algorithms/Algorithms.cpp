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

#include <SudokuLib/SudokuAlgorithm.h>
#include <BoardUtils.h>
#include <SudokuLib/sudokulib_module.h>
#include <SudokuLib/SudokuTypes.h>
#include <SudokuLib/TechniqueMeta.h>
#include <SudokuLib/SudokuPrinter.h>
#include "Combinations.h"

namespace ddahlkvist
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

								u16 with2nodes = 0;
								for (uint i = 0; i < numSharedNodes; ++i)
									with2nodes |= 1u << (begin[i] + 1); // ids are zero based

								matches.push_back({});
								HiddenMatch& m = matches[matches.size() - 1];
								m.numNodes = numSharedNodes;
								m.candidateMask = with2nodes;
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

		RectangleQueryResult canTwoUnitsFormRectangleTwoUnitsOtherDimension(const BitBoard& dimension1, const CandidateBoards9& trialsOtherDimension) {
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
						out[numRectangles] = canTwoUnitsFormRectangleTwoUnitsOtherDimension(mergedBoard, lookup);

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

		struct DimensionQueryResult {
			// only first 27 bits are used, bits { rows [0..8], cols[9..17], blocks[18..26] }
			u32 nodeCountIs1{};
			u32 nodeCountIs2{};

			static constexpr u32 RowBits = 0x1FF;
			static constexpr u32 ColumnBits = 0x3FE00;
			static constexpr u32 BlockBits = 0x7FC0000;
		};

		DimensionQueryResult findDimensionsWhereNodeCountEquals1or2nodes(const SudokuContext& p, const BitBoard& board) {
			DimensionQueryResult result;

			for (uint i = 0; i < 27; ++i) {
				const BitBoard inDimension = p.AllDimensions[i] & board;
				const u32 numBits = inDimension.countSetBits();
				if (numBits == 1) {
					result.nodeCountIs1 |= 1 << i;
				} else if (numBits == 2) {
					result.nodeCountIs2 |= 1 << i;
				}
			}

			return result;
		}

		u8 countColumnsRequired(const SudokuContext& p, const BitBoard& board) {
			u8 count = 0;
			const u8 begin = 9;
			const u8 end = 18;
			for (uint i = begin; i < end; ++i) {
				const BitBoard b = board & p.AllDimensions[i];
				count += b.notEmpty();
			}

			return count;
		}

		void rectangle_foobar(const SudokuContext& p, const BitBoard& board) {
			//struct RectangleMatch {

			//};

			//RectangleMatch matches[100];
			//u8 numMatches = 0;

			//DimensionQueryResult possibleDimensions = findDimensionsWhereNodeCountEquals1or2nodes(p, board);
			//// rows 123 v 789 || 456 v 789 || 123 v 456
			//// 

			//// how can we know that we have 3 nodes that are in a good position? after that, how can we locate the 4th node?
			//	// "any 3 nodes" where exactly 2 nodes share row, and exactly 2 nodes share column
			//	// the 4th node would then be the node that would make it so "any 4 nodes" where exactly 2 nodes share row and exactly 2 nodes share column

			//
			//const auto possibleRows = (possibleDimensions.nodeCountIs1 | possibleDimensions.nodeCountIs2) & possibleDimensions.RowBits;
			//u8 rows[15];
			//const u8 rowCount = fillSetBits(&possibleRows, rows);
			//for (uint i = 0; i < rowCount; ++i) {
			//	const u8 row1 = rows[i];
			//	const bool i_hasOneNode = testBit(possibleDimensions.nodeCountIs1, row1);
			//	const BitBoard row1Nodes = p.AllDimensions[row1] & board;

			//	for (uint j = 0; j < rowCount; ++j) {
			//		const u8 row2 = rows[i];
			//		const bool j_hasOneNode = testBit(possibleDimensions.nodeCountIs2, row2);
			//		
			//		const bool sameBlock = (row1 / 3) == (row2 / 3);
			//		const bool sameNodeCount = i_hasOneNode == j_hasOneNode;
			//		
			//		if (sameBlock || sameNodeCount)
			//			continue;

			//		const BitBoard row2Nodes = p.AllDimensions[row2] & board;
			//		BoardBits::sharesColumn
			//	}
			//}
		}

		bool uniqueRectangle(SudokuContext& p) {
			p.result.Technique = Techniques::UniqueRectangle;

			// if we have 4 nodes, forming a rectangle, over 2 rows AND 2 columns AND 2 blocks [with 2 candidates]
			// if that is the case, we would actually have two possible solutions to the puzzle
			// this is not how any published sudoku is meant to function, meaning we can use this technique to our advantage 
			// so if we know what we are looking for, we can make sure we avoid putting us in that direciton by:
			// any "rectangle where candidates 23 appear in 4 nodes, but one corner has an extra candidate", we know that that corner MUST be that extra candidate

			const BitBoard nodesWith2Candidates = BoardBits::nodesWithCandidateCountBetweenXY(p.AllCandidates, 2, 2);
			rectangle_foobar(p, nodesWith2Candidates);
			return p.result.size() > 0;
		}

		std::vector<TechniqueFunction> allTechniques() {
			std::vector<TechniqueFunction> out = {
				removeNaiveCandidates,
				removeNakedSingle, removeHiddenSingle,
				removeNakedPair, removeNakedTriplet, 
				removeHiddenPair, removeHiddenTriplet, 
				removeNakedQuad, removeHiddenQuad,
				pointingPair,
				boxLineReduction,
				xWing, yWing,
				singleChain,
				uniqueRectangle,
			};
			return out;
		}
	} // techniques
} // dd