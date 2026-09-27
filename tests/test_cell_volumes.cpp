// Regression test for feature_list.txt item 28's Step 1: CellularPotts::
// updateCellVolumes() was rewritten from a Statistics::PixelsByCell()-based
// full pixel-coordinate-list construction (which it then discarded, keeping
// only .size() per cell -- copying the whole grid array and doing an
// indexToPoint()+push_back per occupied pixel just to count them) to a
// direct single-pass tally. This must be behavior-preserving: asserts the
// new tally-based cellVolume values exactly match Statistics::
// PixelsByCell()'s (unchanged, still public) pixel-coordinate counts on the
// same grid state, cross-checking the rewrite against the exact logic it
// replaced rather than just against itself.
#include "test_framework.h"
#include "CellularPotts.h"
#include "Statistics.h"
#include "Parameters.h"

static void buildBlock(CellularPotts& model, int cellId, int x0, int x1, int y0, int y1)
{
	for (int x = x0; x <= x1; x++)
		for (int y = y0; y <= y1; y++)
			model.setPixel(std::pair<int, int>(x, y), cellId);
}

TEST_CASE("updateCellVolumes' direct tally matches Statistics::PixelsByCell()'s pixel-coordinate counts")
{
	Parameters p(2, { {0,20,20},{20,50,20},{20,20,50} }, 20.0f,
	             { 0,5,5 }, { 0,900,900 }, { 0,0,0 }, { 0,0,0 });
	CellularPotts model(std::pair<int, int>(40, 40), &p);

	int idA = model.makeNewCellID(1);
	buildBlock(model, idA, 2, 9, 2, 19); // 8x18 = 144 pixels

	int idB = model.makeNewCellID(2);
	buildBlock(model, idB, 15, 17, 5, 5); // 3x1 = 3 pixels

	int idC = model.makeNewCellID(1);
	buildBlock(model, idC, 30, 30, 30, 30); // single pixel

	model.updateCellVolumes();

	auto expected = Statistics(&model).PixelsByCell();

	CHECK_MESSAGE(model.getCellVolume(idA) == (int)expected[idA].size(),
	              "tallied volume " << model.getCellVolume(idA) << " should match PixelsByCell()'s " << expected[idA].size());
	CHECK_MESSAGE(model.getCellVolume(idB) == (int)expected[idB].size(),
	              "tallied volume " << model.getCellVolume(idB) << " should match PixelsByCell()'s " << expected[idB].size());
	CHECK_MESSAGE(model.getCellVolume(idC) == (int)expected[idC].size(),
	              "tallied volume " << model.getCellVolume(idC) << " should match PixelsByCell()'s " << expected[idC].size());

	CHECK_MESSAGE(model.getCellVolume(idA) == 144, "expected block A's tallied volume to be 144, got " << model.getCellVolume(idA));
	CHECK_MESSAGE(model.getCellVolume(idB) == 3, "expected block B's tallied volume to be 3, got " << model.getCellVolume(idB));
	CHECK_MESSAGE(model.getCellVolume(idC) == 1, "expected block C's tallied volume to be 1, got " << model.getCellVolume(idC));
}

TEST_CASE("updateCellVolumes gives zero volume to a cell ID with no pixels currently painted")
{
	// makeNewCellID's initial placeholder value for a fresh ID's cellVolume
	// entry is not 0 (a pre-existing quirk, unrelated to this change) --
	// this only becomes correct once updateCellVolumes() actually runs.
	Parameters p(1, { {0,20},{20,100} }, 20.0f, { 0,5 }, { 0,900 }, { 0,0 }, { 0,0 });
	CellularPotts model(std::pair<int, int>(20, 20), &p);

	int idA = model.makeNewCellID(1); // allocated, never painted on the grid

	model.updateCellVolumes();

	CHECK_MESSAGE(model.getCellVolume(idA) == 0, "an allocated-but-unpainted cell ID should have zero volume, got " << model.getCellVolume(idA));
}

TEST_MAIN()
