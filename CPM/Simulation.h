#pragma once
#include "Grid.h"
#include "GridManadger.h"
#include "CellularPotts.h"
#include "AdhesionConstraint.h"
#include "VolumeConstraint.h"
#include "PerimeterConstraint.h"
#include "ActivityContraint.h"
#include "PixelsByCell.h"
#include "Centroids.h"
#include <thread>
#include "OpenCL.h"


class Simulation
{

public:

	Simulation(std::vector<std::thread>* threadPool);

	std::vector<std::thread>* threadPool;

	bool runSimulation();
	bool stopSimulation();

	void setupSimulation(int number);

	unsigned char* getImageData();

	std::pair<int, int> getImageSize();

	Parameters p;
	CellularPotts model;

	AdhesionConstraint adhesion;
	VolumeConstraint volume;
	PerimeterConstraint peremiter;
	ActivityContraint activity;

	bool simulationRunning;

	void testFunction();

private:

	OpenCL cl;

	void setupSimulationParallel(CellularPotts* model, bool sameCell, int startIndex, int endIndex, int i, int cellID = -1, int spaceing = 1);



};

