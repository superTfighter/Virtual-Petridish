// Regression tests for CellularPotts::docopy()'s Metropolis acceptance
// criterion -- feature_list.txt item 1 fixed a bug where unfavorable moves
// were accepted via a flat ~50% coin flip regardless of how unfavorable,
// instead of the exp(-deltaH/T) probability. docopy() is private with no
// other public surface to verify it through, so this uses the
// CPM_ENABLE_TEST_HOOKS friend accessor (see CellularPotts.h).
#include "test_framework.h"
#include "CellularPotts.h"
#include "Parameters.h"
#include <cmath>

static CellularPotts makeModel(float temperature)
{
	Parameters p(1, { {0,20},{20,100} }, temperature, { 0,5 }, { 0,1000 }, { 0,0 }, { 0,0 });
	return CellularPotts(std::pair<int, int>(20, 20), &p);
}

TEST_CASE("docopy always accepts favorable moves (deltaH < 0)")
{
	CellularPotts model = makeModel(20.0f);

	int accepted = 0;
	const int trials = 200;
	for (int i = 0; i < trials; i++)
	{
		if (testAccessDocopy(model, -10.0f))
			accepted++;
	}

	CHECK_MESSAGE(accepted == trials, "expected all " << trials << " favorable moves accepted, got " << accepted);
}

TEST_CASE("docopy rejects almost all moves when deltaH is large and positive")
{
	// The exact regression case for the original bug: with T=20 and
	// deltaH=1000, exp(-deltaH/T) = exp(-50) ~ 0 -- essentially never
	// accept. The old buggy implementation accepted ~50% of the time
	// regardless of deltaH's magnitude.
	CellularPotts model = makeModel(20.0f);

	int accepted = 0;
	const int trials = 500;
	for (int i = 0; i < trials; i++)
	{
		if (testAccessDocopy(model, 1000.0f))
			accepted++;
	}

	double acceptRate = (double)accepted / trials;
	CHECK_MESSAGE(acceptRate < 0.05,
	              "accept rate " << acceptRate << " should be near 0, not ~0.5 (the original bug's coin-flip behavior)");
}

TEST_CASE("docopy's acceptance rate approximately follows exp(-deltaH/T)")
{
	const float T = 20.0f;
	const float deltaH = 15.0f; // exp(-15/20) ~= 0.472

	CellularPotts model = makeModel(T);

	int accepted = 0;
	const int trials = 4000;
	for (int i = 0; i < trials; i++)
	{
		if (testAccessDocopy(model, deltaH))
			accepted++;
	}

	double observed = (double)accepted / trials;
	double expected = std::exp(-deltaH / T);

	CHECK_MESSAGE(std::abs(observed - expected) < 0.05,
	              "observed accept rate " << observed << " should be close to exp(-deltaH/T)=" << expected);
}

TEST_MAIN()
