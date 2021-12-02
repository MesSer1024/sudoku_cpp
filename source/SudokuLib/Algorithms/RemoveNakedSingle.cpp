#pragma once

#include <SudokuLib/SudokuAlgorithm.h>
#include <SudokuLib/TechniqueMeta.h>
#include <BoardUtils.h>

namespace ddahlkvist::techniques
{
	bool removeNakedSingle(SudokuContext& p)
	{
		p.result.Technique = Techniques::NakedSingle;

		BitBoard invalid = p.Solved;
		BitBoard touched;

		for (auto&& c : p.AllCandidates) {
			invalid |= (touched & c);
			touched |= c;
		}

		const BitBoard affectedNodes = touched & invalid.invert();
		if (affectedNodes.notEmpty()) {
			p.result.storePreModification(p.b.Nodes, affectedNodes);

			affectedNodes.foreachSetBit([&p](u32 bitIndex) {
				const u16 candidate = getOnlyCandidateFromMask(p.b.Nodes[bitIndex].getCandidates());
				p.b.Nodes[bitIndex].solve(candidate);
				});
		}

		return p.result.size() > 0;
	}

}