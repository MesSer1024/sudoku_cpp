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
		}

	private:
		std::vector<Change> Changes;
		BitBoard _dirty;
	};

	namespace techniques
	{
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
			// foreach solved node in cell ...

			bool modified = false;
			BitBoard solved = BoardBits::bitsSolved(b);
			BitBoard unsolved = BoardBits::bitsUnsolved(b);
			
			{
				const BoardBits::BitBoards9 rows = BoardBits::AllRows();
				for (uint i = 0; i < 9; ++i)
				{
					const u32 solvedValues = buildValueMaskFromSolvedNodes(b.Nodes, solved & rows[i]);
					if (solvedValues && solvedValues != Candidates::All)
					{
						const u16 mask = toCandidateMask(solvedValues);
						BitBoard modifiedNodes = wouldRemoveCandidates(b.Nodes, unsolved & rows[i], mask);
						if (modifiedNodes.hasValues())
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
						if (modifiedNodes.hasValues())
						{
							result.storePreModification(b.Nodes, modifiedNodes);
							removeCandidatesForNodes(b.Nodes, modifiedNodes, mask);
						}
					}
				}
			}
			{
				const BoardBits::BitBoards9 cells = BoardBits::AllCells();
				for (uint i = 0; i < 9; ++i)
				{
					const u32 solvedValues = buildValueMaskFromSolvedNodes(b.Nodes, solved & cells[i]);
					if (solvedValues && solvedValues != Candidates::All)
					{
						const u16 mask = toCandidateMask(solvedValues);
						BitBoard modifiedNodes = wouldRemoveCandidates(b.Nodes, unsolved & cells[i], mask);
						if (modifiedNodes.hasValues())
						{
							result.storePreModification(b.Nodes, modifiedNodes);
							removeCandidatesForNodes(b.Nodes, modifiedNodes, mask);
						}
					}
				}
			}

			return modified;
		}

		bool soloCandidate(const Board& b, Result& outResult)
		{
			u8 i = 0;
			for (Node n : b.Nodes)
			{
				std::bitset<9> candidates(n.getCandidates());
				if (candidates.count() == 1)
				{
					unsigned long bitIndex;
					_BitScanForward(&bitIndex, candidates.to_ulong());
					outResult.append(n, i);
					n.solve(bitIndex);
					return true;
				}
				i++;
			}
			return false;
		}
	}
}