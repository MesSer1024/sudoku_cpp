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

	enum class TechniqueUsed {
		None = 0,
		NaiveCandidates,
		NakedSingle,
		HiddenSingle,
		NakedPair,
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
			Technique = TechniqueUsed::None;
		}

		TechniqueUsed Technique { TechniqueUsed::None };
	private:
		std::vector<Change> Changes;
		BitBoard _dirty;
	};

	namespace techniques
	{
		//---------------- Sudoku Solver -------------
		//Check for solved cells
		//Show Possibles	No

		//		x: Basic Candidates				removeNaiveCandidates() [All solved neighbours toMask -> remove as candidates on unsolved]
		//		0: Naked Single					removeNakedSingle() [ All unsolved nodes -> If a node only has one candidate, it can be solved]
		//		1: Hidden Singles				removeHiddenSingle() [ All unsolved neighbours -> if this is only occurance of candidate, it can be solved]
		//		2: Naked Pairs/Triples			removeNakedPair() // [ All unsolved in dimension -> if 2 nodes only have 2 candidates and share them, all other nodes in dimension can remove those candidates]

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

		void fillAllUnsolvedWithAllCandidates(Board& b)
		{
			BitAction applyAllCandidatesAction = [&b](u32 bitIndex) {
				b.Nodes[bitIndex].candidatesSet(Candidates::All);
			};

			BoardBits::SudokuBitBoard unsolvedNodes = BoardBits::bitsUnsolved(b);
			unsolvedNodes.foreachSetBit(applyAllCandidatesAction);
		}

		bool removeNaiveCandidates(Board& b, Result& result)
		{
			// foreach solved node in row
			//		build mask of solved values and invert relevant bits to generate mask of possible candidates
			//		
			//		locate all nodes in row that would have a candidate removed by this candidateMask (to know which ones are truly modified)
			//		store the previous values for all nodes that are about to change in "result"
			//		remove any candidates for affected nodes
			// foreach solved node in column ...
			// foreach solved node in block ...

			// might be possible to do in parallell... but job is probably too small to be useful to parallell
			// build all possible masks and what nodes they should affect and store in "storage"
			// merge results
			// apply results

			const BitBoard solved = BoardBits::bitsSolved(b);
			const BitBoard unsolved = BoardBits::bitsUnsolved(b);
			
			{
				const BoardBits::BitBoards9 rows = BoardBits::AllRows();
				for (uint i = 0; i < 9; ++i)
				{
					const u32 solvedValues = buildValueMaskFromSolvedNodes(b.Nodes, solved & rows[i]);
					if (solvedValues && solvedValues != Candidates::All)
					{
						const u16 mask = toCandidateMask(solvedValues);
						BitBoard modifiedNodes = wouldRemoveCandidates(b.Nodes, unsolved & rows[i], mask);
						if (modifiedNodes.notEmpty())
						{
							result.storePreModification(b.Nodes, modifiedNodes);
							removeCandidatesForNodes(b.Nodes, modifiedNodes, mask);
						}
					}
				}
			}
			{
				const BoardBits::BitBoards9 cols = BoardBits::AllColumns();
				for (uint i = 0; i < 9; ++i)
				{
					const u32 solvedValues = buildValueMaskFromSolvedNodes(b.Nodes, solved & cols[i]);
					if (solvedValues && solvedValues != Candidates::All)
					{
						const u16 mask = toCandidateMask(solvedValues);
						BitBoard modifiedNodes = wouldRemoveCandidates(b.Nodes, unsolved & cols[i], mask);
						if (modifiedNodes.notEmpty())
						{
							result.storePreModification(b.Nodes, modifiedNodes);
							removeCandidatesForNodes(b.Nodes, modifiedNodes, mask);
						}
					}
				}
			}
			{
				const BoardBits::BitBoards9 blocks = BoardBits::AllBlocks();
				for (uint i = 0; i < 9; ++i)
				{
					const u32 solvedValues = buildValueMaskFromSolvedNodes(b.Nodes, solved & blocks[i]);
					if (solvedValues && solvedValues != Candidates::All)
					{
						const u16 mask = toCandidateMask(solvedValues);
						BitBoard modifiedNodes = wouldRemoveCandidates(b.Nodes, unsolved & blocks[i], mask);
						if (modifiedNodes.notEmpty())
						{
							result.storePreModification(b.Nodes, modifiedNodes);
							removeCandidatesForNodes(b.Nodes, modifiedNodes, mask);
						}
					}
				}
			}

			return result.size() > 0;
		}

		bool removeNakedSingle(Board& b, Result& outResult)
		{
			BitBoard affectedNodes;
			BitBoard unsolved = BoardBits::bitsUnsolved(b);
			unsolved.foreachSetBit([&b, &affectedNodes](u32 bitIndex) {
				if (countCandidates(b.Nodes[bitIndex].getCandidates()) == 1) {
					affectedNodes.setBit(bitIndex);
				}
			});

			if (affectedNodes.notEmpty()) {
				outResult.storePreModification(b.Nodes, affectedNodes);
				affectedNodes.foreachSetBit([&b](u32 bitIndex) {
					b.Nodes[bitIndex].solve(getOnlyCandidateFromMask(b.Nodes[bitIndex].getCandidates()));
				});
				return true;
			}
			return false;
		}

		// [ All unsolved in dimension -> if candidate only exists in one node in dimension, it can be solved]
		bool removeHiddenSingle(Board& b, Result& result) {
			const BoardBits::BitBoards9 candidates = buildCandidateBoards(b);
			const BoardBits::BitBoards27 allDirections = BoardBits::AllDimensions();

			BitBoard unhandledNodes(BitBoard::All{});
			u8 affectedNodes[BoardSize];
			u8 targetValue[BoardSize];
			u8 numAffectedNodes = 0;

			for (uint c = 0; c < 9; ++c) {
				for (uint i = 0; i < 27; ++i) {
					// for all candidates , all possible directions, find unsolved with that candidate that we have not already fixed
					const BitBoard maskedNodesWithCandidate = allDirections[i] & candidates[c];
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
					result.append(b.Nodes[nodeId], nodeId);
				}
				for (uint i = 0; i < numAffectedNodes; ++i) {
					const u8 nodeId = affectedNodes[i];
					const u8 value = targetValue[i];
					b.Nodes[nodeId].solve(value);
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
		bool removeNakedPair(Board& b, Result& result) {
			const BoardBits::BitBoards9 candidates = buildCandidateBoards(b);
			const BoardBits::BitBoards27 allDirections = BoardBits::AllDimensions();
			
			NakedPairMatch matches[256];
			u8 numMatches = 0;



			for (auto&& dimension : allDirections) {
				BoardBits::BitBoards9 potentialNodes;
				u16 potentialCandidateId[9];
				u8 numPotentials = 0;

				// check if any candidate is shared by exacty two nodes
				for(u8 candidateId=0; candidateId < 9; ++candidateId) {
					const BitBoard cd = dimension & candidates[candidateId];
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
				for (uint i = 0; i < numMatches; ++i) {
					const NakedPairMatch& match = matches[i];

					const BitBoard sharedNeighbours = BoardBits::SharedNeighboursClearSelf(match.node1, match.node2);
					const BitBoard affectedNodes = (candidates[match.candidateId1] | candidates[match.candidateId2]) & sharedNeighbours;

					if (affectedNodes.notEmpty()) {
						// save node-state before modification
						result.storePreModification(b.Nodes, affectedNodes);

						// modify the neighbours that have these candidates
						const u16 combinedMask = match.getCombinedCandidateValueMask();
						affectedNodes.foreachSetBit([&b, combinedMask](u32 bitIndex) {
							b.Nodes[bitIndex].candidatesRemoveBySolvedMask(combinedMask);
						});
					}
				}
			}

			return result.size() > 0;
		}
	}
}