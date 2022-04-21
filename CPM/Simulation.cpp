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

		/*

		//START THREADS
		for (size_t i = 0; i < calc_thread.size(); i++)
		{

			calc_thread[i] = std::thread(&Simulation::setupSimulationParallel, this, &model, true, init_i,init_i+block_size,0, cellId,-1);

			init_i = init_i + block_size;
		}

		//WAIT FOR THREADS TO FINISH
		for (size_t i = 0; i < calc_thread.size(); i++)
		{
			if (calc_thread[i].joinable())
				calc_thread[i].join();
			else
			{
				i = i - 1;
				std::this_thread::sleep_for(std::chrono::microseconds(1));
			}
		}

		*/



		model.addConstraint(&adhesion);

	}
	else if (number == 2) { //EPHILIA

		srand(time(NULL));
		p = Parameters(1, { {0,0},{0,-2} }, 50.0f, { 0,70 }, { 0,100 }, { 0,3 }, { 0,0 });

		model = CellularPotts(std::pair<int, int>(1000, 300), &p);

		int init_i = 0;
		int block_size = this->model.grid.size.first / calc_thread.size();

		//START THREADS
		for (size_t i = 0; i < calc_thread.size(); i++)
		{

			calc_thread[i] = std::thread(&Simulation::setupSimulationParallel, this,&model, false, init_i, init_i + block_size, 0,-1,50);

			init_i = init_i + block_size;
		}

		//WAIT FOR THREADS TO FINISH
		for (size_t i = 0; i < calc_thread.size(); i++)
		{
			if (calc_thread[i].joinable())
				calc_thread[i].join();
			else
			{
				i = i - 1;
				std::this_thread::sleep_for(std::chrono::microseconds(1));
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

		p = Parameters(1, { {0,20},{20,100} }, 20.0f, { 0,2 }, { 0,100 }, { 0,0 }, { 0,0 });
		model = CellularPotts(std::pair<int, int>(100, 100), &p);

		model.setPixel(std::pair<int, int>(10, 25), model.makeNewCellID(1));
		model.setPixel(std::pair<int, int>(10, 25), model.makeNewCellID(1));
		model.setPixel(std::pair<int, int>(20, 25), model.makeNewCellID(1));

		model.addConstraint(&adhesion);
		model.addConstraint(&volume);
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

void Simulation::setupSimulationParallel(CellularPotts* model,bool sameCell, int startIndex, int endIndex, int i, int cellID, int spaceing)
{
	
	if(sameCell)
	{

		for (size_t x = startIndex; x < endIndex; x++)
		{
			for (size_t j = 0; j < model->grid.size.second; j++)
			{
				float randomNumber = 0.0f + (float)(rand()) / ((float)(RAND_MAX / (1.0f - 0.0f)));

				if (randomNumber < 0.49) {
					model->setPixel(std::pair<int, int>(x, j), cellID);
				}
			}
		}
	}
	else
	{

		for (size_t x = startIndex; x < endIndex; x+= spaceing)
		{
			for (size_t j = 5; j < model->grid.size.second; j += spaceing)
			{
				model->setPixel(std::pair<int, int>(x, j), model->makeNewCellID(1));
			}

		}

	}



}

