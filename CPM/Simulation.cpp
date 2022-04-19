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
	if (number == 0)
	{
		srand(time(NULL));

		p = Parameters(1, { {0,20},{20,100} }, 20.0f, { 0,5 }, { 0,1000 }, { 0,0 }, { 0,0 });
		model = CellularPotts(std::pair<int, int>(500, 500), &p);

		model.setPixel(std::pair<int, int>(model.grid.size.first / 2, model.grid.size.second / 2), model.makeNewCellID(1));

		model.addConstraint(&adhesion);
		model.addConstraint(&volume);
	}
	else if (number == 1)
	{
		srand(time(NULL));

		p = Parameters(3, { {0,20},{20,100} }, 20.0f, { 0,5 }, { 0,10 }, { 0,0 }, { 0,0 });

		model = CellularPotts(std::pair<int, int>(500, 500), &p);

		auto cellId = model.makeNewCellID(1);

		for (size_t i = 0; i < model.grid.size.first; i++)
		{
			for (size_t j = 0; j < model.grid.size.second; j++)
			{
				float randomNumber = 0.0f + (float)(rand()) / ((float)(RAND_MAX / (1.0f - 0.0f)));

				if (randomNumber < 0.49) {
					model.setPixel(std::pair<int, int>(i, j), cellId);
				}
			}
		}

		std::cout << "Init finished" << std::endl;

		model.addConstraint(&adhesion);

	}
	else if (number == 2) {

		srand(time(NULL));
		p = Parameters(1, { {0,0},{0,-2} }, 50.0f, { 0,70 }, { 0,100 }, { 0,3 }, { 0,0 });

		model = CellularPotts(std::pair<int, int>(1000, 300), &p);

		for (size_t i = 5; i < 1000; i += 50)
		{
			for (size_t j = 5; j < 300; j += 50)
			{
				model.setPixel(std::pair<int, int>(i, j), model.makeNewCellID(1));
			}

		}

		model.addConstraint(&adhesion);
		model.addConstraint(&volume);
		model.addConstraint(&peremiter);


	}
	else if (number == 3) {

		//TODO:CellSorting
	}
	else if (number == 4) {

		srand(time(NULL));

		p = Parameters(1, { {0,20},{20,100} }, 20.0f, { 0,20 }, { 0,2000 }, { 0,0 }, { 0,0 });
		model = CellularPotts(std::pair<int, int>(500, 500), &p);


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

		p = Parameters(1, { {0,20},{20,100} }, 20.0f, { 0,2 }, { 0,100 }, { 0,0 }, { 0,0 });
		model = CellularPotts(std::pair<int, int>(1000, 500), &p);

		model.setPixel(std::pair<int, int>(10, 250), model.makeNewCellID(1));
		model.setPixel(std::pair<int, int>(10, 250), model.makeNewCellID(1));
		model.setPixel(std::pair<int, int>(20, 250), model.makeNewCellID(1));

		model.addConstraint(&adhesion);
		model.addConstraint(&volume);
	}
	else if (number == 6) {

		//TODO:DIVISION

		//TODO: REMOVE ACT MODEL
		srand(time(NULL));


		p = Parameters(1, { {0,20},{20,100} }, 20.0f, { 0,50 }, { 0,500 }, { 0,2 }, { 0,260 },"geometric", { 0,300 }, { 0,30 });

		model = CellularPotts(std::pair<int, int>(250, 250), &p);

		model.setPixel(std::pair<int, int>(model.grid.size.first / 2, model.grid.size.second / 2), model.makeNewCellID(1));

		model.addConstraint(&adhesion);
		model.addConstraint(&volume);
		model.addConstraint(&activity);

	}

}

unsigned char* Simulation::getImageData()
{

	if(this->model.parameters->ACT_MEAN != "false")
	{
		return model.getRenderImage(activity.cellpixelsact);
	}
	else
	{
		return model.getRenderImage();
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
