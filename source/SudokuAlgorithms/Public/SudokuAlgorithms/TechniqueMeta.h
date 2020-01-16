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
	};

//namespace techniques
//{
//	bool removeNakedSingle(SudokuContext& p);
//	bool removeNakedPair(SudokuContext& p);
//	bool removeNakedTriplet(SudokuContext& p);
//	bool removeNakedQuad(SudokuContext& p);
//	bool removeHiddenSingle(SudokuContext& p);
//	bool removeHiddenPair(SudokuContext& p);
//	bool removeHiddenTriplet(SudokuContext& p);
//	bool removeHiddenQuad(SudokuContext& p);
//	bool pointingPair(SudokuContext& p);
//}
//
//
//
//using TechniqueFunction = std::function<bool(SudokuContext& p)>;
//using TechniqueFunctionPair = std::pair<Techniques, TechniqueFunction>;
//std::map<Techniques, TechniqueFunction> TechniqueFunctionLookup = {
//	TechniqueFunctionPair(Techniques::NakedSingle, techniques::removeNakedSingle),
//};

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

};

}