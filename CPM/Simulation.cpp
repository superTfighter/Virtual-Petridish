#include "Simulation.h"

Simulation::Simulation(std::vector<std::thread>* threadPool)
{
	this->threadPool = threadPool;
	simulationRunning = false;

}

bool Simulation::runSimulation()
{

	if (!simulationRunning)
	{
		int i = 0;

		bool running = false;

		for (auto it = threadPool->begin(); it != threadPool->end(); ++it)
		{

			if (!running) {

				threadPool->operator[](i) = std::thread(&CellularPotts::monteCarloParallel, &model);

				running = true;
			}

			i++;
		}

		simulationRunning = true;
	}

	return simulationRunning;
}

bool Simulation::stopSimulation()
{
	if (simulationRunning)
	{
		model.stopRequested = true;

		// runSimulation() only ever populates threadPool[0]; join it before
		// letting a subsequent runSimulation() reassign a std::thread that's
		// still joinable, which would call std::terminate().
		if (threadPool->at(0).joinable())
			threadPool->at(0).join();

		simulationRunning = false;
	}

	return !simulationRunning;
}

void Simulation::setupSimulation(int number)
{
	//PARALLEL STUFF
	std::vector<std::thread> calc_thread = std::vector<std::thread>(4);


	if (number == 0)
	{
		srand(time(NULL));

		p = Parameters(1, { {0,20},{20,100} }, 20.0f, { 0,5 }, { 0,1000 }, { 0,0 }, { 0,0 });
		model = CellularPotts(std::pair<int, int>(500, 500), &p);

		model.setPixel(std::pair<int, int>(model.grid.size.first / 2, model.grid.size.second / 2), model.makeNewCellID(1));

		model.addConstraint(&adhesion);
		model.addConstraint(&volume);
	}
	else if (number == 1) //ISING
	{
		srand(time(NULL));

		p = Parameters(1, { {0,20},{20,100} }, 20.0f, { 0,5 }, { 0,10 }, { 0,0 }, { 0,0 });

		model = CellularPotts(std::pair<int, int>(250, 250), &p);

		auto cellId = model.makeNewCellID(1);


		for (size_t x = 0; x < model.grid.size.first; x++)
		{
			for (size_t j = 0; j < model.grid.size.second; j++)
			{
				float randomNumber = 0.0f + (float)(rand()) / ((float)(RAND_MAX / (1.0f - 0.0f)));

				if (randomNumber < 0.49) {
					model.setPixel(std::pair<int, int>(x, j), cellId);
				}
			}
		}

		model.addConstraint(&adhesion);

	}
	else if (number == 2) { //EPHILIA

		srand(time(NULL));
		p = Parameters(1, { {0,0},{0,-2} }, 50.0f, { 0,5 }, { 0,100 }, { 0,3 }, { 0,40 });

		model = CellularPotts(std::pair<int, int>(500, 500), &p);

		for (size_t x = 0; x < model.grid.size.first; x += 10)
		{
			for (size_t j = 0; j < model.grid.size.second; j += 10)
			{
				model.setPixel(std::pair<int, int>(x, j), model.makeNewCellID(1));
			}

		}

		model.addConstraint(&adhesion);
		model.addConstraint(&volume);
		model.addConstraint(&peremiter);

	}
	else if (number == 3) { //CELLSORTING

		srand(time(NULL));
		p = Parameters(2, { {0,12,6},{12,6,16},{6,16,6} }, 15.0f, { 0,2,2 }, { 0,25,25 }, { 0,0 }, { 0,0 });

		model = CellularPotts(std::pair<int, int>(500, 500), &p);

		int n = 500;
		int max_attempts = 10 * n;

		for (int i = 0; i < n; i++)
		{
			int kind = (i % 2 == 0) ? 1 : 2; // intermix the two kinds 1:1

			for (int attempt = 0; attempt < max_attempts; attempt++)
			{
				std::pair<int, int> point(rand() % model.grid.size.first, rand() % model.grid.size.second);

				if (model.grid.pixti(model.grid.pointToIndex(point)) == 0)
				{
					model.setPixel(point, model.makeNewCellID(kind));
					break;
				}
			}
		}

		model.addConstraint(&adhesion);
		model.addConstraint(&volume);
	}
	else if (number == 4) {

		srand(time(NULL));

		p = Parameters(1, { {0,20},{20,100} }, 20.0f, { 0,200 }, { 0,2000 }, { 0,0 }, { 0,0 });
		model = CellularPotts(std::pair<int, int>(1000, 1000), &p);

		int numCells = 7;
		int max_attempts = 1000;

		for (int i = 0; i < numCells; i++)
		{
			for (int attempt = 0; attempt < max_attempts; attempt++)
			{
				std::pair<int, int> point(rand() % model.grid.size.first, rand() % model.grid.size.second);

				if (model.grid.pixti(model.grid.pointToIndex(point)) == 0)
				{
					model.setPixel(point, model.makeNewCellID(1));
					break;
				}
			}
		}

		model.addConstraint(&adhesion);
		model.addConstraint(&volume);
	}
	else if (number == 5) { //WOUND HEALING

		srand(time(NULL));

		p = Parameters(1, { {0,100},{100,-100} }, 20.0f, { 0,150 }, { 0,625 }, { 0,0 }, { 0,0 },
		               "geometric", { 0,50 }, { 0,50 });
		model = CellularPotts(std::pair<int, int>(500, 500), &p);

		// Two dense near-confluent bands (rows y=0..79 and y=420..499) with a
		// real ~340px gap between them -- a wound, not just an empty field.
		for (size_t x = 0; x < 500; x += 5)
		{
			for (size_t j = 0; j < 80; j += 5)
			{
				model.setPixel(std::pair<int, int>(x, j), model.makeNewCellID(1));
			}
			for (size_t j = 420; j < 500; j += 5)
			{
				model.setPixel(std::pair<int, int>(x, j), model.makeNewCellID(1));
			}
		}

		model.addConstraint(&adhesion);
		model.addConstraint(&volume);
		model.addConstraint(&activity);

		model.cellDivision = true;

	}
	else if (number == 6) { //PERIMETER DEMO

		srand(time(NULL));

		p = Parameters(1, { {0,20},{20,100} }, 20.0f, { 0,10 }, { 0,900 }, { 0,4 }, { 0,120 });
		model = CellularPotts(std::pair<int, int>(500, 500), &p);

		int numCells = 6;
		int max_attempts = 1000;

		for (int i = 0; i < numCells; i++)
		{
			for (int attempt = 0; attempt < max_attempts; attempt++)
			{
				std::pair<int, int> point(rand() % model.grid.size.first, rand() % model.grid.size.second);

				if (model.grid.pixti(model.grid.pointToIndex(point)) == 0)
				{
					model.setPixel(point, model.makeNewCellID(1));
					break;
				}
			}
		}

		model.addConstraint(&adhesion);
		model.addConstraint(&volume);
		model.addConstraint(&peremiter);
	}
	else if (number == 7) { //ADHESION + MIGRATION

		srand(time(NULL));

		p = Parameters(2, { {0,12,6},{12,6,16},{6,16,6} }, 15.0f, { 0,2,2 }, { 0,25,25 }, { 0,0 }, { 0,0 },
		               "geometric", { 0,40,40 }, { 0,40,40 });

		model = CellularPotts(std::pair<int, int>(500, 500), &p);

		int n = 300;
		int max_attempts = 10 * n;

		for (int i = 0; i < n; i++)
		{
			int kind = (i % 2 == 0) ? 1 : 2;

			for (int attempt = 0; attempt < max_attempts; attempt++)
			{
				std::pair<int, int> point(rand() % model.grid.size.first, rand() % model.grid.size.second);

				if (model.grid.pixti(model.grid.pointToIndex(point)) == 0)
				{
					model.setPixel(point, model.makeNewCellID(kind));
					break;
				}
			}
		}

		model.addConstraint(&adhesion);
		model.addConstraint(&volume);
		model.addConstraint(&activity);
	}
	else if (number == 8) { //CELL DIVISION

		srand(time(NULL));

		p = Parameters(1, { {0,20},{20,-100} }, 20.0f, { 0,15 }, { 0,1500 }, { 0,0 }, { 0,0 });
		model = CellularPotts(std::pair<int, int>(1000, 1000), &p);

		model.setPixel(std::pair<int, int>(model.grid.size.first / 2, model.grid.size.second / 2), model.makeNewCellID(1));

		model.addConstraint(&adhesion);
		model.addConstraint(&volume);

		model.cellDivision = true;

	}
	else if (number == 9) { //NUTRIENT FORAGING

		srand(time(NULL));

		p = Parameters(1, { {0,20},{20,100} }, 20.0f, { 0,10 }, { 0,300 }, { 0,0 }, { 0,0 });
		p.CONSUMPTION_RATE = { 0,5 };

		model = CellularPotts(std::pair<int, int>(500, 500), &p);

		int numCells = 10;
		int max_attempts = 1000;

		for (int i = 0; i < numCells; i++)
		{
			for (int attempt = 0; attempt < max_attempts; attempt++)
			{
				std::pair<int, int> point(rand() % model.grid.size.first, rand() % model.grid.size.second);

				if (model.grid.pixti(model.grid.pointToIndex(point)) == 0)
				{
					model.setPixel(point, model.makeNewCellID(1));
					break;
				}
			}
		}

		model.addConstraint(&adhesion);
		model.addConstraint(&volume);
		model.addConstraint(&eating);
	}
	else if (number == 10) { //PREDATION

		srand(time(NULL));

		// kind 1 = predator, kind 2 = prey. Baseline adhesion is mild/symmetric;
		// PredationConstraint is what actually drives predator-into-prey
		// invasion once they're in contact (see PredationConstraint::deltaH).
		// Predators also get ActivityContraint migration (prey doesn't -- its
		// LAMBDA_ACT/MAX_ACT are 0) so they actively hunt instead of relying
		// on growth radius alone to ever reach a prey cell. Predator volume
		// target (5000) is well above what they can realistically reach given
		// the grid/region size, so they stay hungry and keep hunting for a
		// long time rather than satiating early and going passive -- but it's
		// bounded (unlike an unreachable target), so they don't instantly
		// consume the entire grid the moment the scenario starts.
		p = Parameters(2, { {0,20,20},{20,100,20},{20,20,100} }, 20.0f,
		               { 0,15,10 }, { 0,5000,150 }, { 0,0,0 }, { 0,0,0 },
		               "geometric", { 0,60,0 }, { 0,60,0 });
		p.PREDATOR_OF = { 0,2,0 }; // kind 1 preys on kind 2, kind 2 preys on nothing

		model = CellularPotts(std::pair<int, int>(300, 300), &p);

		int numPredators = 8;
		int numPrey = 30;
		int max_attempts = 1000;

		// Both kinds are seeded into the same central region (not scattered
		// across the whole grid) so predators and prey actually end up close
		// enough to meet -- otherwise growth+migration alone rarely brings a
		// sparse, randomly-scattered predator into contact with any prey
		// within a reasonable time.
		int regionMin = 90, regionMax = 210;

		for (int i = 0; i < numPredators; i++)
		{
			for (int attempt = 0; attempt < max_attempts; attempt++)
			{
				std::pair<int, int> point(regionMin + rand() % (regionMax - regionMin), regionMin + rand() % (regionMax - regionMin));

				if (model.grid.pixti(model.grid.pointToIndex(point)) == 0)
				{
					model.setPixel(point, model.makeNewCellID(1));
					break;
				}
			}
		}

		for (int i = 0; i < numPrey; i++)
		{
			for (int attempt = 0; attempt < max_attempts; attempt++)
			{
				std::pair<int, int> point(regionMin + rand() % (regionMax - regionMin), regionMin + rand() % (regionMax - regionMin));

				if (model.grid.pixti(model.grid.pointToIndex(point)) == 0)
				{
					model.setPixel(point, model.makeNewCellID(2));
					break;
				}
			}
		}

		model.addConstraint(&adhesion);
		model.addConstraint(&volume);
		model.addConstraint(&predation);
		model.addConstraint(&activity);
	}
	else if (number == 11) { //SUBSTATES DEMO

		srand(time(NULL));

		// A single kind (1) with 2 substates that behave like different
		// kinds via Parameters::stateIndex: state 0 = "small" (low volume
		// target, weak self-adhesion), state 1 = "grown" (much larger
		// volume target, stronger self-adhesion). Rows/cols 0-1 are unused
		// placeholders (medium and the never-occurring kind0/state1 slot);
		// real cells only ever use indices 2 (kind1,state0) and 3
		// (kind1,state1).
		p = Parameters(1, {
			{0,0,20,20},
			{0,0,0,0},
			{20,0,50,30},
			{20,0,30,50}
		}, 20.0f, { 0,0,10,10 }, { 0,0,100,600 }, { 0,0,0,0 }, { 0,0,0,0 });
		p.NUM_STATES = 2;

		model = CellularPotts(std::pair<int, int>(500, 500), &p);

		int numCells = 10;
		int max_attempts = 1000;

		for (int i = 0; i < numCells; i++)
		{
			for (int attempt = 0; attempt < max_attempts; attempt++)
			{
				std::pair<int, int> point(rand() % model.grid.size.first, rand() % model.grid.size.second);

				if (model.grid.pixti(model.grid.pointToIndex(point)) == 0)
				{
					int id = model.makeNewCellID(1);
					model.setCellState(id, i % 2); // alternate state 0 / state 1
					model.setPixel(point, id);
					break;
				}
			}
		}

		model.addConstraint(&adhesion);
		model.addConstraint(&volume);
	}
	else if (number == 12) { //SANDBOX

		srand(time(NULL));

		p = Parameters(3, { {0,20,20,20},{20,50,20,20},{20,20,50,20},{20,20,20,50} }, 20.0f,
		               { 0,5,5,5 }, { 0,500,500,500 }, { 0,0,0,0 }, { 0,0,0,0 });
		model = CellularPotts(std::pair<int, int>(500, 500), &p);

		model.addConstraint(&adhesion);
		model.addConstraint(&volume);

		model.cellDivision = false;
	}

	std::cout << "Setup done" << std::endl;
}

unsigned char* Simulation::getImageData()
{

	if(this->model.parameters->ACT_MEAN != "false")
	{

		return cl.getRenderImage(&this->model, &this->activity);
		
	}
	else
	{
		return cl.getRenderImage(&this->model);
	}

}

std::pair<int, int> Simulation::getImageSize()
{
	return this->model.grid.size;
}

void Simulation::testFunction()
{
	std::cout << " DONE" << std::endl;
}


