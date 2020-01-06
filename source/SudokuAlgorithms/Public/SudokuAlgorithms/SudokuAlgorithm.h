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
	struct Change
	{
		u8 index;
		Node prev;
	};

	enum class Techniques {
		None = 0,
		NaiveCandidates,
		NakedSingle,
		HiddenSingle,
		NakedPair,
		NakedTriplet,
	};

	struct Result
	{
		void storePreModification(const Node* nodes, const BitBoard& affectedNodes)
		{
			BitBoard newDirty = (affectedNodes ^ _dirty) & affectedNodes;
			_dirty |= affectedNodes;

			newDirty.foreachSetBit([nodes, &Changes = Changes](u32 bitIndex) {
				Changes.push_back({ static_cast<u8>(bitIndex), nodes[bitIndex] });
			});
		}

		void append(Node old, u8 id)
		{
			if(!_dirty.test(id))
				Changes.push_back({ id, old });
		}

		Change fetch(uint idx)
		{
			return Changes[idx];
		}

		uint size() {
			return static_cast<uint>(Changes.size());
		}

		bool operator()() {
			return Changes.size() > 0;
		}

		BitBoard pullDirty() {
			return _dirty;
		}

		void reset() {
			Changes.clear();
			_dirty = {};
			Technique = Techniques::None;
		}

		Techniques Technique { Techniques::None };
	private:
		std::vector<Change> Changes;
		BitBoard _dirty;
	};

	struct SudokuContext {
		//SudokuContext(Board& board, Result& r)
		//	: b(board)
		//	, result(r)
		//{}
		Board& b;
		Result& result;

		const BitBoard Solved;
		const BitBoard Unsolved;
		const BoardBits::BitBoards9 AllCandidates;
		const BoardBits::BitBoards27 AllDimensions;
	};

	namespace techniques
	{
		//---------------- Sudoku Solver -------------
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

			for (auto&& board : p.AllDimensions) {
				const u16 mask = toCandidateMask(buildValueMaskFromSolvedNodes(p.b.Nodes, p.Solved & board));
				if (mask && mask != Candidates::All) {
					BitBoard modifiedNodes = wouldRemoveCandidates(p.b.Nodes, p.Unsolved & board, mask);
					if (modifiedNodes.notEmpty())
					{
						p.result.storePreModification(p.b.Nodes, modifiedNodes);

						removeCandidatesForNodes(p.b.Nodes, modifiedNodes, mask);
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
						if (maskedNodesWithCandidate.size() == 1) {
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

		struct NakedPairMatch {
			u16 candidateIds[4]{};
			u8 nodeIds[4]{};

			u16 getCombinedCandidateValueMask(u32 depth) const {
				u16 mask = buildValueMaskFromCandidateIds(&candidateIds[0], depth);
				return mask;
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

				candidatesWithTwoNodesInDimension = dimensionalNodes;
				savedCandidateId = candidateIds;

				std::vector<int> v(numPotentials);
				std::iota(v.begin(), v.end(), 0);

				for_each_combination(v.begin(), v.begin() + depth, v.end(), [&](auto a, auto b) -> bool {
					return test(a,b);
				});
			}

			template <class It>
			bool test(const It a, const It b)
			{
				It curr = a;
				BitBoard prev = candidatesWithTwoNodesInDimension[*curr];
				while (++curr != b) {
					if (!(prev == candidatesWithTwoNodesInDimension[*curr])) {
						return false;
					}
				}

				u8 nodes[4];
				assert(prev.fillSetBits(nodes) == depth);

				// should be 0..Depth
				for (uint i = 0; i < depth; ++i) {
					foundMatches[numMatchesNew].nodeIds[i] = nodes[i];
					foundMatches[numMatchesNew].candidateIds[i] = savedCandidateId[a[i]];
				}

				numMatchesNew++;
				return false;
			}

			u32 fillMatches(NakedPairMatch* matches) {
				for (uint i = 0; i < numMatchesNew; ++i) {
					matches[i] = foundMatches[i];
				}
				return numMatchesNew;
			}

		private:
			BitBoard* candidatesWithTwoNodesInDimension{ nullptr };
			u16* savedCandidateId{ nullptr };
			NakedPairMatch foundMatches[64];
			u8 depth{ 2 };
			u8 numMatchesNew{ 0 };
		};

		// [ All unsolved in dimension -> if 2 nodes only have 2 candidates and they are the same candidates, all other nodes in dimension can remove those candidates]
		bool removeNakedPair(SudokuContext& p) {
			p.result.Technique = Techniques::NakedPair;

			const uint depth = 2;
			PermutationSolver solver(depth);

			NakedPairMatch matches[256];
			u32 numMatches = 0;

			for (auto&& dimension : p.AllDimensions) {
				BoardBits::BitBoards9 candidatesWithTwoNodesInDimension;
				u16 savedCandidateId[9];
				u8 numPotentials = 0;

				// check if any candidate is shared by exactly two nodes
				for (u8 candidateId = 0; candidateId < 9; ++candidateId) {
					const BitBoard cd = dimension & p.AllCandidates[candidateId];
					const bool ExactlyDepthNodesShareCandidate = cd.size() == depth;
					if (ExactlyDepthNodesShareCandidate) {
						savedCandidateId[numPotentials] = candidateId;
						candidatesWithTwoNodesInDimension[numPotentials] = cd;

						numPotentials++;
					}
				}

				// see if two candidate-matches share the exact nodes
				if(numPotentials >= depth) {
					solver.run(numPotentials, candidatesWithTwoNodesInDimension.data(), savedCandidateId);
					numMatches += solver.fillMatches(&matches[numMatches]);
				}
			}

			// see if any neighbours to the nodes located have any of marked candidates
			if (numMatches > 0) {
				assert(numMatches < 256);
				for (uint i = 0; i < numMatches; ++i) {
					const NakedPairMatch& match = matches[i];

					const BitBoard sharedNeighboursNew = BoardBits::DistinctNeighboursClearSelf(match.nodeIds, depth);
					const BitBoard sharedNeighbours = BoardBits::SharedNeighboursClearSelf(match.nodeIds[0], match.nodeIds[1]);
					assert(sharedNeighboursNew == sharedNeighbours);
					const BitBoard sharedCandidateBoard = (p.AllCandidates[match.candidateIds[0]] | p.AllCandidates[match.candidateIds[1]]);
					const BitBoard affectedNodes = sharedCandidateBoard & sharedNeighbours;

					if (affectedNodes.notEmpty()) {
						// save node-state before modification
						p.result.storePreModification(p.b.Nodes, affectedNodes);

						// modify the neighbours that have these candidates
						const u16 combinedMask = match.getCombinedCandidateValueMask(depth);
						affectedNodes.foreachSetBit([&p, combinedMask](u32 bitIndex) {
							p.b.Nodes[bitIndex].candidatesRemoveBySolvedMask(combinedMask);
						});
					}
				}
			}

			return p.result.size() > 0;
		}

		bool removeNakedTriplet(SudokuContext& p) {
			p.result.Technique = Techniques::NakedTriplet;

			return p.result.size() > 0;
		}
	}
}