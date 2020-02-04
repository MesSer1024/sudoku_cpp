#pragma once
#include <functional>
#include <map>

#include <SudokuAlgorithms/SudokuTypes.h>
#include <SudokuAlgorithms/SudokuAlgorithm.h>

namespace dd
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

using TechniqueName = std::pair<Techniques, const char*>;
std::map<Techniques, const char*> TechniqueNameLookup = {
	TechniqueName(Techniques::None, "NA"),
	TechniqueName(Techniques::NaiveCandidates, "Naive"),
	TechniqueName(Techniques::NakedSingle, "Naked Single"),
	TechniqueName(Techniques::HiddenSingle, "Hidden Single"),
	TechniqueName(Techniques::NakedPair, "Naked Pair"),
	TechniqueName(Techniques::NakedTriplet, "Naked Triplet"),
	TechniqueName(Techniques::HiddenPair, "Hidden Pair"),
	TechniqueName(Techniques::HiddenTriplet, "Hidden Triplet"),
	TechniqueName(Techniques::NakedQuad, "Naked Quad"),
	TechniqueName(Techniques::NakedQuad, "Hidden Quad"),
	TechniqueName(Techniques::PointingPair, "PointingPair"),
	TechniqueName(Techniques::BoxLineReduction, "BoxLineReduction"),
	TechniqueName(Techniques::X_Wing, "X-Wing"),
	TechniqueName(Techniques::Y_Wing, "Y-Wing"),
	TechniqueName(Techniques::SingleChain, "Single Chain"),
	TechniqueName(Techniques::UniqueRectangle, "Unique Rectangle"),
};

}