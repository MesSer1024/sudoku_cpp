#pragma once

#include <SudokuLib/SudokuAlgorithm.h>
#include <SudokuLib/TechniqueMeta.h>
#include <BoardUtils.h>
#include "Combinations.h"

namespace ddahlkvist::techniques
{

	struct HiddenMatch {
		u8 nodes[4]{};
		u16 candidateMask;
		u8 numNodes{};
	};

	bool removeHiddenInternal(std::vector<HiddenMatch>& matches, const SudokuContext& p, u8 depth)
	{
		for (auto&& dimension : p.AllDimensions) {
			const BitBoard unsolvedDimension = dimension & p.Unsolved;
			const u8 numUnsolvedInDimension = unsolvedDimension.countSetBits();
			if (numUnsolvedInDimension <= depth) {
				// early out if tehcnique would not do anything
				continue;
			}

			auto transformWhereCountIsDepth = [](u8* outArray, u8* inArray, u8 depth) -> u8 {
				u8 numPotentials = 0;

				for (u8 i = 0; i < 9; ++i) {
					const u8 value = inArray[i];
					if (value >= 2 && value <= depth)
						outArray[numPotentials++] = i;
				}

				return numPotentials;
			};

			u8 numNodesWithCandidate[9];
			std::vector<u8> candidateIdsWithSufficientNodes(9);

			BoardUtils::countTimesEachCandidateOccur(numNodesWithCandidate, p, unsolvedDimension);
			const u8 numPotentials = transformWhereCountIsDepth(candidateIdsWithSufficientNodes.data(), numNodesWithCandidate, depth);

			const bool techniqueCouldWork = numPotentials >= depth && numPotentials < numUnsolvedInDimension;
			if (techniqueCouldWork) {
				// foreach_candidate
				// merge bitboards for [a, b, c] (depth3) if they share same candidates, we have match

				auto begin = candidateIdsWithSufficientNodes.begin();
				for_each_combination(begin, begin + depth, begin + numPotentials, [&p, &unsolvedDimension, &matches](auto a, auto end) {
					const auto begin = a;
					const u8 depth = static_cast<u8>(end - a);
					BitBoard merged;
					while (a != end) {
						merged |= p.AllCandidates[*a];
						a++;
					}

					const BitBoard combo = merged & unsolvedDimension;
					const u8 numSharedNodes = combo.countSetBits();
					if (numSharedNodes >= 2 && numSharedNodes <= depth)
					{
						// validate that the nodes have more candidates than these
						u8 nodes[9];
						combo.fillSetBits(nodes);

						const u16 allNodeCandidates = BoardUtils::mergeCandidateMasks(p, nodes, numSharedNodes);
						const u16 numSharedCandidates = countCandidates(allNodeCandidates);
						if (numSharedCandidates > numSharedNodes) {

							u16 with2nodes = 0;
							for (uint i = 0; i < numSharedNodes; ++i)
								with2nodes |= 1u << (begin[i] + 1); // ids are zero based

							matches.push_back({});
							HiddenMatch& m = matches[matches.size() - 1];
							m.numNodes = numSharedNodes;
							m.candidateMask = with2nodes;
							for (uint i = 0; i < numSharedNodes; ++i)
								m.nodes[i] = nodes[i];
						}
					}

					return false;
					});
			}
		}

		return !matches.empty();
	}

	// hidden implies that a DIMENSION lacks CANDIDATE, and only a few nodes can have that value, if 2 candidate can only exist in 2 nodes, those values MUST be in those nodes
	bool removeHiddenPair(SudokuContext& p) {
		p.result.Technique = Techniques::HiddenPair;
		std::vector<HiddenMatch> matches;
		const u8 depth = 2;

		removeHiddenInternal(matches, p, depth);
		// in dimension, foreach candidate, count how many nodes have this candidate
		// if bitCount of candidates are shared by a few amount of nodes

		if (!matches.empty())
		{
			for (auto&& match : matches) {
				for (uint i = 0; i < match.numNodes; ++i) {
					const u8 nodeId = match.nodes[i];
					Node& node = p.b.Nodes[nodeId];
					p.result.append(node, nodeId);

					node.candidatesSet(match.candidateMask);
				}

			}

			return p.result.size() > 0;
		}
		return false;
	}

	bool removeHiddenTriplet(SudokuContext& p) {
		p.result.Technique = Techniques::HiddenTriplet;
		std::vector<HiddenMatch> matches;
		const u8 depth = 3;

		removeHiddenInternal(matches, p, depth);
		// in dimension, foreach candidate, count how many nodes have this candidate
		// if bitCount of candidates are shared by a few amount of nodes

		if (!matches.empty())
		{
			for (auto&& match : matches) {
				for (uint i = 0; i < match.numNodes; ++i) {
					const u8 nodeId = match.nodes[i];
					Node& node = p.b.Nodes[nodeId];
					p.result.append(node, nodeId);

					node.candidatesSet(match.candidateMask);
				}

			}

			return p.result.size() > 0;
		}
		return false;
	}

	bool removeHiddenQuad(SudokuContext& p) {
		p.result.Technique = Techniques::HiddenQuad;
		std::vector<HiddenMatch> matches;
		const u8 depth = 4;

		removeHiddenInternal(matches, p, depth);
		// in dimension, foreach candidate, count how many nodes have this candidate
		// if bitCount of candidates are shared by a few amount of nodes

		if (!matches.empty())
		{
			for (auto&& match : matches) {
				for (uint i = 0; i < match.numNodes; ++i) {
					const u8 nodeId = match.nodes[i];
					Node& node = p.b.Nodes[nodeId];
					p.result.append(node, nodeId);

					node.candidatesSet(match.candidateMask);
				}

			}

			return p.result.size() > 0;
		}
		return false;
	}
}