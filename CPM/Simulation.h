#pragma once
#include "Grid.h"
#include "GridManadger.h"
#include "CellularPotts.h"
#include "AdhesionConstraint.h"
#include "VolumeConstraint.h"
#include "PerimeterConstraint.h"
#include "ActivityContraint.h"
#include "EatingConstraint.h"
#include "PredationConstraint.h"
#include "ResourceSeekingConstraint.h"
#include "ChemotaxisConstraint.h"
#include "PixelsByCell.h"
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
	EatingConstraint eating;
	PredationConstraint predation;
	ResourceSeekingConstraint resourceSeeking;
	ChemotaxisConstraint chemotaxis;

	bool simulationRunning;

	void testFunction();

private:

	OpenCL cl;

};

