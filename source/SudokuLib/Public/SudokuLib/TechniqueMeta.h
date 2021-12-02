#pragma once
#include <functional>
#include <map>

#include <SudokuLib/SudokuTypes.h>
#include <SudokuLib/SudokuAlgorithm.h>

namespace ddahlkvist
{
	struct SudokuContext;

	enum class Techniques {
		None = 0,
		NaiveCandidates,
		NakedSingle,
		HiddenSingle,
		NakedPair,
		NakedTriplet,
		HiddenPair,
		HiddenTriplet,
		NakedQuad,
		HiddenQuad,
		PointingPair,
		BoxLineReduction,
		X_Wing, 
		Y_Wing,
		SingleChain,
		UniqueRectangle,
	};

}