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
	if (!simulationRunning)
	{
		//TODO: STOP THIS
		simulationRunning = false;


	}

	return !simulationRunning;
}

void Simulation::setupSimulation(int number)
{
	//PARALELL STUFF
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

		p = Parameters(3, { {0,20},{20,100} }, 20.0f, { 0,5 }, { 0,10 }, { 0,0 }, { 0,0 });

		model = CellularPotts(std::pair<int, int>(100, 100), &p);

		int init_i = 0;
		int block_size = this->model.grid.size.first / calc_thread.size();

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
		p = Parameters(1, { {0,0},{0,-2} }, 50.0f, { 0,700 }, { 0,15000 }, { 0,3 }, { 0,0 });

		model = CellularPotts(std::pair<int, int>(500, 500), &p);

		int init_i = 0;
		int block_size = this->model.grid.size.first / calc_thread.size();


		for (size_t x = 0; x < model.grid.size.first; x += 10)
		{
			for (size_t j = 0; j < model.grid.size.second; j += 10)
			{
				model.setPixel(std::pair<int, int>(x, j), model.makeNewCellID(1));
			}

		}


		model.addConstraint(&adhesion);
		model.addConstraint(&volume);


	}
	else if (number == 3) {

		//TODO:CellSorting
	}
	else if (number == 4) {

		srand(time(NULL));

		p = Parameters(1, { {0,20},{20,100} }, 20.0f, { 0,200 }, { 0,2000 }, { 0,0 }, { 0,0 });
		model = CellularPotts(std::pair<int, int>(1000, 1000), &p);


		model.setPixel(std::pair<int, int>(100, 100), model.makeNewCellID(1));
		model.setPixel(std::pair<int, int>(100, 250), model.makeNewCellID(1));
		model.setPixel(std::pair<int, int>(200, 250), model.makeNewCellID(1));
		model.setPixel(std::pair<int, int>(250, 100), model.makeNewCellID(1));
		model.setPixel(std::pair<int, int>(300, 250), model.makeNewCellID(1));
		model.setPixel(std::pair<int, int>(350, 300), model.makeNewCellID(1));
		model.setPixel(std::pair<int, int>(450, 450), model.makeNewCellID(1));


		model.addConstraint(&adhesion);
		model.addConstraint(&volume);
	}
	else if (number == 5) {

		srand(time(NULL));

		p = Parameters(1, { {0,20},{20,-100} }, 20.0f, { 0,200 }, { 0,10000 }, { 0,0 }, { 0,0 });
		model = CellularPotts(std::pair<int, int>(250, 250), &p);

		for (size_t i = 0; i < model.grid.size.first; i+=10)
		{

			model.setPixel(std::pair<int, int>(i, 125), model.makeNewCellID(1));

		}

	



		model.addConstraint(&adhesion);
		model.addConstraint(&volume);
		model.addConstraint(&peremiter);

	}
	else if (number == 6) {

		//TODO:DIVISION

		//TODO: REMOVE ACT MODEL
		srand(time(NULL));


		p = Parameters(1, { {0,20},{20,100} }, 20.0f, { 0,50 }, { 0,500 }, { 0,2 }, { 0,260 },"arithmetic", { 0,300 }, { 0,300 });

		model = CellularPotts(std::pair<int, int>(1000, 1000), &p);

		model.setPixel(std::pair<int, int>(model.grid.size.first / 2, model.grid.size.second / 2), model.makeNewCellID(1));

		model.addConstraint(&adhesion);
		model.addConstraint(&volume);
		model.addConstraint(&activity);

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


