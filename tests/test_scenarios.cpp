// Regression test for feature_list.txt item 14: several demo scenarios were
// found broken (a literal unfinished stub, a self-deadlock, a crash on cell
// death) purely by running them. This runs every registered scenario
// (Simulation::setupSimulation cases 0-12) for a handful of Monte Carlo
// steps and checks nothing throws -- the same "does it even run" smoke
// check that caught those bugs, now checked in instead of thrown away.
#include "test_framework.h"
#include "Simulation.h"
#include <stdexcept>

static const int NUM_SCENARIOS = 13; // cases 0-12, see Simulation::setupSimulation
static const int STEPS_PER_SCENARIO = 20;

TEST_CASE("every demo scenario runs several Monte Carlo steps without throwing")
{
	for (int scenario = 0; scenario < NUM_SCENARIOS; scenario++)
	{
		std::vector<std::thread> threadPool(2);
		Simulation sim(&threadPool);

		bool threw = false;
		std::string what;

		try
		{
			sim.setupSimulation(scenario);

			for (int step = 0; step < STEPS_PER_SCENARIO; step++)
				sim.model.monteCarloStep();
		}
		catch (std::exception& e)
		{
			threw = true;
			what = e.what();
		}
		catch (...)
		{
			threw = true;
			what = "non-std::exception thrown";
		}

		CHECK_MESSAGE(!threw, "scenario " << scenario << " threw: " << what);
	}
}

TEST_MAIN()
