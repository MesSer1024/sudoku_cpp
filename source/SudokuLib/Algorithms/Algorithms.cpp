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

namespace ddahlkvist::techniques
{
	// each of the different techniques are residing within their own file that is forwarded from this point

	std::vector<TechniqueFunction> allTechniques() {
		std::vector<TechniqueFunction> out = {
			removeNaiveCandidates,
			removeNakedSingle, removeHiddenSingle,
			removeNakedPair, removeNakedTriplet,
			removeHiddenPair, removeHiddenTriplet,
			removeNakedQuad, removeHiddenQuad,
			removePointingPair,
			removeBoxLineReduction,
			removeXWing, removeYWing,
			removeSingleChain,
			removeUniqueRectangle,
		};
		return out;
	}
}