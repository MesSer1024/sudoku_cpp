#pragma once

#include <SudokuLib/SudokuAlgorithm.h>
#include <SudokuLib/TechniqueMeta.h>
#include <BoardUtils.h>

namespace ddahlkvist::techniques
{
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

}