#pragma once

#include <SudokuLib/SudokuAlgorithm.h>
#include <SudokuLib/TechniqueMeta.h>
#include <BoardUtils.h>

namespace ddahlkvist::techniques
{


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
}