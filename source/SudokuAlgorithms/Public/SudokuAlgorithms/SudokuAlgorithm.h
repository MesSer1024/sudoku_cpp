#pragma once

#include <bitset>
#include <intrin.h>
#include <map>
#include <vector>

#include <SudokuAlgorithms/BoardUtils.h>
#include <SudokuAlgorithms/Module.h>
#include <SudokuAlgorithms/SudokuTypes.h>

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

			//const BoardBits::BitBoards9 candidates = buildCandidateBoards(b);
			//const BoardBits::BitBoards27 allDirections = BoardBits::AllDimensions();
			//BitBoard affectedNodes;

			//for (auto&& dir : allDirections) {
			//	for (auto&& c : candidates) {
			//		const BitBoard candidateInDimension = dir & c;
			//		if (candidateInDimension.count() == 1u) {
			//			const u8 nodeId = candidateInDimension.firstOne();
			//			affectedNodes.setBit(nodeId);
			//		}
			//	}
			//}

			BitBoard unhandledNodes(BitBoard::All{});
			u8 affectedNodes[BoardSize];
			u8 targetValue[BoardSize];
			u8 numAffectedNodes = 0;

			for (uint c = 0; c < 9; ++c) {
				for (uint i = 0; i < 27; ++i) {
					// for all candidates , all possible directions, find unsolved with that candidate that we have not already fixed
					const BitBoard maskedNodesWithCandidate = p.AllDimensions[i] & p.AllCandidates[c];
					if (maskedNodesWithCandidate.count() == 1) {
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
			u8 node1;
			u8 node2;
			u16 candidateId1;
			u16 candidateId2;

			u16 getCombinedCandidateValueMask() const { return buildValueMask(candidateId1 + 1, candidateId2 + 1); }
		};

		// [ All unsolved in dimension -> if 2 nodes only have 2 candidates and they are the same candidates, all other nodes in dimension can remove those candidates]
		bool removeNakedPair(SudokuContext& p) {
			p.result.Technique = Techniques::NakedPair;

			NakedPairMatch matches[256];
			u16 numMatches = 0;

			for (auto&& dimension : p.AllDimensions) {
				BoardBits::BitBoards9 potentialNodes;
				u16 potentialCandidateId[9];
				u8 numPotentials = 0;

				// check if any candidate is shared by exacty two nodes
				for(u8 candidateId=0; candidateId < 9; ++candidateId) {
					const BitBoard cd = dimension & p.AllCandidates[candidateId];
					const bool ExactlyTwoNodesShareCandidate = cd.count() == 2u;
					if (ExactlyTwoNodesShareCandidate) {
						potentialCandidateId[numPotentials] = candidateId;
						potentialNodes[numPotentials] = cd;
						
						numPotentials++;
					}
				}

				// check if we have two candidates shared by exactly two nodes
				if (numPotentials >= 2) {
					for (uint i = 0; i < numPotentials; ++i) {
						for (uint j = i + 1; j < numPotentials; ++j) {
							const bool sharedBySameNodes = potentialNodes[i] == (potentialNodes[i] & potentialNodes[j]);
							if (sharedBySameNodes) {
								u8 bothNodes[4];
								assert(potentialNodes[i].fillSetBits(bothNodes) == 2u);

								matches[numMatches].node1 = bothNodes[0];
								matches[numMatches].node2 = bothNodes[1];
								matches[numMatches].candidateId1 = potentialCandidateId[i];
								matches[numMatches].candidateId2 = potentialCandidateId[j];

								numMatches++;
							}
						}
					}
				}
			}

			// see if any neighbours to the nodes located have any of marked candidates
			if (numMatches > 0) {
				assert(numMatches < 256);
				for (uint i = 0; i < numMatches; ++i) {
					const NakedPairMatch& match = matches[i];

					const BitBoard sharedNeighbours = BoardBits::SharedNeighboursClearSelf(match.node1, match.node2);
					const BitBoard affectedNodes = (p.AllCandidates[match.candidateId1] | p.AllCandidates[match.candidateId2]) & sharedNeighbours;

					if (affectedNodes.notEmpty()) {
						// save node-state before modification
						p.result.storePreModification(p.b.Nodes, affectedNodes);

						// modify the neighbours that have these candidates
						const u16 combinedMask = match.getCombinedCandidateValueMask();
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