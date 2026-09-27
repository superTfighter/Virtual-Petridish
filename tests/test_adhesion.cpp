// Regression test for AdhesionConstraint's kind-vs-cell-ID indexing bug
// (feature_list.txt item 2): H() used to convert a neighbor's raw cell ID
// to its kind before comparing it against the still-raw target-type ID,
// so whether two same-kind pixels were (wrongly) flagged as "different"
// depended on whether the specific numeric ID happened to coincide with a
// kind value -- i.e. results were sensitive to which arbitrary IDs cells
// happened to have, not just their kinds. Adhesion energy must depend only
// on kind, never on the specific ID values involved.
#include "test_framework.h"
#include "CellularPotts.h"
#include "AdhesionConstraint.h"
#include "Parameters.h"
#include <cmath>

// Builds a 3x3 block of one cell (kind 1) with one invader pixel of another
// cell (kind 2) adjacent to it, then returns AdhesionConstraint::deltaH for
// the invader growing one more pixel into the block's territory.
// `idOffset` burns that many placeholder cell IDs first (allocated via
// makeNewCellID but never painted on the grid) so the block/invader end up
// with different specific ID values across calls, while the kind
// configuration stays identical -- isolating "does the result depend on
// kind only" from "does it depend on the raw ID values too".
static float computeDeltaHForSetup(int idOffset)
{
	Parameters p(2, { {0,20,20},{20,50,30},{20,30,50} }, 20.0f,
	             { 0,5,5 }, { 0,900,900 }, { 0,0,0 }, { 0,0,0 });
	CellularPotts model(std::pair<int, int>(20, 20), &p);

	AdhesionConstraint adhesion;
	model.addConstraint(&adhesion);

	for (int i = 0; i < idOffset; i++)
		model.makeNewCellID(1); // burn IDs, never painted on the grid

	int blockId = model.makeNewCellID(1);
	for (int x = 8; x <= 10; x++)
		for (int y = 8; y <= 10; y++)
			model.setPixel(std::pair<int, int>(x, y), blockId);

	int invaderId = model.makeNewCellID(2);
	model.setPixel(std::pair<int, int>(11, 9), invaderId); // adjacent to the block's right edge

	int sourceI = model.grid.pointToIndex(std::pair<int, int>(11, 9));
	int targetI = model.grid.pointToIndex(std::pair<int, int>(10, 9)); // block's right-edge pixel

	return adhesion.deltaH(sourceI, targetI, invaderId, blockId);
}

TEST_CASE("AdhesionConstraint::deltaH depends only on kind, not on the specific cell ID values used")
{
	float deltaH_lowIds = computeDeltaHForSetup(0);   // block/invader get IDs 1,2
	float deltaH_highIds = computeDeltaHForSetup(10); // block/invader get IDs 11,12

	CHECK_MESSAGE(std::abs(deltaH_lowIds - deltaH_highIds) < 0.001f,
	              "deltaH with low IDs (" << deltaH_lowIds << ") should equal deltaH with high IDs ("
	              << deltaH_highIds << ") -- adhesion energy must depend only on kind, not raw ID values");
}

TEST_MAIN()
