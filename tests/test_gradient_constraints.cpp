// Directional-correctness regression tests for the two gradient-following
// constraints added this session (feature_list.txt item 24):
// ResourceSeekingConstraint and ChemotaxisConstraint. Both went through
// rounds of "compiles and runs" that turned out to have the wrong sign or a
// gradient too weak to matter at real range -- verified at the time only via
// throwaway headless scripts, using a full stochastic monteCarloStep() loop
// and measuring net centroid drift. That approach was tried here first and
// turned out to be too noisy to check in: CellularPotts's own RNG usage
// (rand(), never re-seeded between the two models these tests construct)
// produces enough centroid random-walk on its own to swamp the constraints'
// bias within any step count fast enough to keep this test suite quick.
// Instead these call deltaH() directly for known pixel/resource/scent
// configurations and check its SIGN (and relative ordering, e.g. a bigger
// gap should never flip the sign) -- deterministic, fast, and a direct
// regression test for the exact "wrong sign" bug class that occurred.
// Magnitude/LAMBDA constants are deliberately NOT pinned here: both
// constraints' LAMBDA needed several retuning passes this session already
// (see the constraints' own header comments), so a test asserting an exact
// value would need updating every time someone retunes it for gameplay feel.
#include "test_framework.h"
#include "CellularPotts.h"
#include "ResourceSeekingConstraint.h"
#include "ChemotaxisConstraint.h"
#include "Parameters.h"

TEST_CASE("ResourceSeekingConstraint::deltaH favors moving into higher-resource pixels")
{
	Parameters p(1, { {0,20},{20,100} }, 20.0f, { 0,5 }, { 0,100 }, { 0,0 }, { 0,0 });
	p.SEEK_RESOURCES = true;

	CellularPotts model(std::pair<int, int>(20, 20), &p);

	ResourceSeekingConstraint resourceSeeking;
	model.addConstraint(&resourceSeeking);

	int cellId = model.makeNewCellID(1);
	model.setPixel(std::pair<int, int>(10, 10), cellId);

	int sourceI = model.grid.pointToIndex(std::pair<int, int>(10, 10));
	int nearI = model.grid.pointToIndex(std::pair<int, int>(10, 11));  // small resource boost
	int farI = model.grid.pointToIndex(std::pair<int, int>(10, 12));   // bigger resource boost
	int sameI = model.grid.pointToIndex(std::pair<int, int>(10, 9));   // left untouched -- same as source

	model.grid.addResourceAt(nearI, 20.0f);
	model.grid.addResourceAt(farI, 80.0f);

	float deltaTowardSmallGap = resourceSeeking.deltaH(sourceI, nearI, cellId, 0);
	float deltaTowardBigGap = resourceSeeking.deltaH(sourceI, farI, cellId, 0);
	float deltaTowardNoGap = resourceSeeking.deltaH(sourceI, sameI, cellId, 0);
	float deltaAwayFromGap = resourceSeeking.deltaH(nearI, sourceI, cellId, 0); // reversed source/target

	CHECK_MESSAGE(deltaTowardSmallGap < 0, "moving into a richer pixel should be favorable (negative), got " << deltaTowardSmallGap);
	CHECK_MESSAGE(deltaTowardBigGap < 0, "moving into an even richer pixel should be favorable (negative), got " << deltaTowardBigGap);
	CHECK_MESSAGE(deltaTowardBigGap < deltaTowardSmallGap, "a bigger resource gap (" << deltaTowardBigGap << ") should be more favorable than a smaller one (" << deltaTowardSmallGap << ")");
	CHECK_MESSAGE(deltaTowardNoGap == 0.0f, "equal resource on both pixels should give exactly zero bias, got " << deltaTowardNoGap);
	CHECK_MESSAGE(deltaAwayFromGap > 0, "moving away from a richer pixel (reversed source/target) should be unfavorable (positive), got " << deltaAwayFromGap);
}

TEST_CASE("ResourceSeekingConstraint::deltaH is gated by SEEK_RESOURCES and by a non-cell source")
{
	Parameters p(1, { {0,20},{20,100} }, 20.0f, { 0,5 }, { 0,100 }, { 0,0 }, { 0,0 });
	p.SEEK_RESOURCES = false; // gate off

	CellularPotts model(std::pair<int, int>(20, 20), &p);

	ResourceSeekingConstraint resourceSeeking;
	model.addConstraint(&resourceSeeking);

	int cellId = model.makeNewCellID(1);
	model.setPixel(std::pair<int, int>(10, 10), cellId);

	int sourceI = model.grid.pointToIndex(std::pair<int, int>(10, 10));
	int targetI = model.grid.pointToIndex(std::pair<int, int>(10, 11));
	model.grid.addResourceAt(targetI, 100.0f); // would otherwise be a strongly favorable move

	CHECK_MESSAGE(resourceSeeking.deltaH(sourceI, targetI, cellId, 0) == 0.0f,
	              "SEEK_RESOURCES == false should disable the bias entirely regardless of the resource gap");

	p.SEEK_RESOURCES = true;

	CHECK_MESSAGE(resourceSeeking.deltaH(sourceI, targetI, 0, cellId) == 0.0f,
	              "a non-cell (medium) source_type should never get a resource-seeking bias -- only real cells move toward resources");
}

TEST_CASE("ChemotaxisConstraint::deltaH favors moving toward closer-to-prey pixels")
{
	// numberOfCells=2 -> J/V/etc indexed 0=medium, 1=hunter, 2=prey.
	Parameters p(2, { {0,20,20},{20,50,20},{20,20,50} }, 20.0f,
	             { 0,5,5 }, { 0,100,100 }, { 0,0,0 }, { 0,0,0 });
	p.PREDATOR_OF = { 0,2,0 }; // kind 1 hunts kind 2

	CellularPotts model(std::pair<int, int>(60, 20), &p);

	ChemotaxisConstraint chemotaxis;
	model.addConstraint(&chemotaxis); // afterSetModelMethod sizes+refreshes preyScent (all zero, no prey placed yet)

	int hunterId = model.makeNewCellID(1);
	model.setPixel(std::pair<int, int>(10, 10), hunterId);

	int preyId = model.makeNewCellID(2);
	model.setPixel(std::pair<int, int>(50, 10), preyId);

	// The scent field only refreshes every REFRESH_INTERVAL (10) MCS calls
	// (see ChemotaxisConstraint.cpp) -- call postMCSListener() that many
	// times to force a refresh reflecting the prey placed above.
	for (int i = 0; i < 10; i++)
		chemotaxis.postMCSListener();

	int nearPreyI = model.grid.pointToIndex(std::pair<int, int>(21, 10)); // 29px from prey
	int farPreyI = model.grid.pointToIndex(std::pair<int, int>(20, 10));  // 30px from prey (one step further away)

	float deltaTowardPrey = chemotaxis.deltaH(farPreyI, nearPreyI, hunterId, 0);
	float deltaAwayFromPrey = chemotaxis.deltaH(nearPreyI, farPreyI, hunterId, 0);

	CHECK_MESSAGE(deltaTowardPrey < 0, "moving one step closer to prey should be favorable (negative), got " << deltaTowardPrey);
	CHECK_MESSAGE(deltaAwayFromPrey > 0, "moving one step away from prey should be unfavorable (positive), got " << deltaAwayFromPrey);
}

TEST_CASE("ChemotaxisConstraint::deltaH is gated by PREDATOR_OF and by a non-hunter source")
{
	Parameters p(2, { {0,20,20},{20,50,20},{20,20,50} }, 20.0f,
	             { 0,5,5 }, { 0,100,100 }, { 0,0,0 }, { 0,0,0 });
	p.PREDATOR_OF = { 0,2,0 }; // kind 1 hunts kind 2, kind 2 hunts nothing

	CellularPotts model(std::pair<int, int>(60, 20), &p);

	ChemotaxisConstraint chemotaxis;
	model.addConstraint(&chemotaxis);

	int hunterId = model.makeNewCellID(1);
	model.setPixel(std::pair<int, int>(10, 10), hunterId);

	int preyId = model.makeNewCellID(2);
	model.setPixel(std::pair<int, int>(50, 10), preyId);

	for (int i = 0; i < 10; i++)
		chemotaxis.postMCSListener();

	int nearPreyI = model.grid.pointToIndex(std::pair<int, int>(21, 10));
	int farPreyI = model.grid.pointToIndex(std::pair<int, int>(20, 10));

	CHECK_MESSAGE(chemotaxis.deltaH(farPreyI, nearPreyI, preyId, 0) == 0.0f,
	              "prey's own kind (PREDATOR_OF[2] == 0) should never get a hunting bias");

	CHECK_MESSAGE(chemotaxis.deltaH(farPreyI, nearPreyI, 0, hunterId) == 0.0f,
	              "a non-cell (medium) source_type should never get a hunting bias");
}

TEST_MAIN()
