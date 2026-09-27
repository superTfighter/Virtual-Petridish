#pragma once
#include <string>
#include <iostream>
#include <vector>

class Parameters
{
public:

	Parameters();

	Parameters(int numberOfCells, std::vector<std::vector<int>> J, float T, std::vector<float> LAMBDA_V, std::vector<float> V, std::vector<float> LAMBDA_P, std::vector<float> P)
	{
		this->numberOfCells = numberOfCells;

		this->J = J;
		this->T = T;

		this->LAMBDA_V = LAMBDA_V;
		this->V = V;

		this->LAMBDA_P = LAMBDA_P;
		this->P = P;

		ACT_MEAN = "false";
		LAMBDA_ACT = std::vector<float>();
		MAX_ACT = std::vector<float>();

		this->LAMDA_DIR = std::vector<float>();
		this->PERSIST = std::vector<float>();
	}

	Parameters(int numberOfCells, std::vector<std::vector<int>> J, float T, std::vector<float> LAMBDA_V, std::vector<float> V, std::vector<float> LAMBDA_P, std::vector<float> P, std::string ACT_MEAN, std::vector<float> LAMBDA_ACT, std::vector<float> MAX_ACT)
	{
		this->numberOfCells = numberOfCells;

		this->J = J;
		this->T = T;

		this->LAMBDA_V = LAMBDA_V;
		this->V = V;

		this->LAMBDA_P = LAMBDA_P;
		this->P = P;

		this->ACT_MEAN = ACT_MEAN;
		this->LAMBDA_ACT = LAMBDA_ACT;
		this->MAX_ACT = MAX_ACT;

		this->LAMDA_DIR = std::vector<float>();
		this->PERSIST = std::vector<float>();
	}

	Parameters(int numberOfCells, std::vector<std::vector<int>> J, float T, std::vector<float> LAMBDA_V, std::vector<float> V, std::vector<float> LAMBDA_P, std::vector<float> P, std::string ACT_MEAN, std::vector<float> LAMBDA_ACT, std::vector<float> MAX_ACT, std::vector<float> LAMDA_DIR, std::vector<float> PERSIST)
	{
		this->numberOfCells = numberOfCells;

		this->J = J;
		this->T = T;

		this->LAMBDA_V = LAMBDA_V;
		this->V = V;

		this->LAMBDA_P = LAMBDA_P;
		this->P = P;

		this->ACT_MEAN = ACT_MEAN;
		this->LAMBDA_ACT = LAMBDA_ACT;
		this->MAX_ACT = MAX_ACT;

		this->LAMDA_DIR = LAMDA_DIR;
		this->PERSIST = PERSIST;
	}

	//Adhesion parameter , Multidimensional vector => [[0,20],[20,100]] => means 0:0 => 0 , 0:1 => 20, 1:0 => 20 , 1:1 => 100
	std::vector<std::vector<int>> J;

	//Temperature parameter
	float T;

	//How many cellkinds do we have
	int numberOfCells;

	// Volume constraint parameters per cellKind
	std::vector<float> LAMBDA_V;
	std::vector<float> V;

	// Perimeter constraint parameters per cellKind
	std::vector<float> LAMBDA_P;
	std::vector<float> P;

	//ACTIVITY PARAMETERS
	std::string ACT_MEAN;
	std::vector<float> LAMBDA_ACT;
	std::vector<float> MAX_ACT;

	//PERSISTENCE PARAMETERS
	std::vector<float> LAMDA_DIR;
	std::vector<float> PERSIST;

	// EATING/RESOURCE PARAMETERS -- per-cell-kind resource consumption rate.
	// Not part of any constructor (default-constructs empty like the other
	// arrays would if omitted); scenarios that use EatingConstraint set it
	// directly after construction, the same way Simulation::setupSimulation
	// sets model.cellDivision post-construction.
	std::vector<float> CONSUMPTION_RATE;

	// PREDATION PARAMETERS -- PREDATOR_OF[kind] = the kind this kind preys
	// on (0 = none / not a predator). Same post-construction-assignment
	// convention as CONSUMPTION_RATE above.
	std::vector<int> PREDATOR_OF;

	// SUBSTATE PARAMETERS -- NUM_STATES is how many substates each kind has
	// (default 1, via the in-class initializer below, so every existing
	// constructor/scenario gets it "for free" without touching any of
	// them). stateIndex(kind, state) flattens a (kind, state) pair into a
	// single index for constraints that want per-(kind,state) parameter
	// arrays (V, LAMBDA_V, J, ...) instead of per-kind ones: when
	// NUM_STATES==1, stateIndex(kind, 0) == kind, so every existing
	// scenario's kind-indexed arrays keep working completely unchanged. A
	// scenario that wants real substates sets NUM_STATES>1 and sizes its
	// arrays to numKinds*NUM_STATES entries, addressed via stateIndex.
	int NUM_STATES = 1;
	int stateIndex(int kind, int state) const { return kind * NUM_STATES + state; }

	// GRADIENT-FOLLOWING PARAMETERS -- runtime-mutable toggles (default
	// off), same post-construction convention as CONSUMPTION_RATE/
	// PREDATOR_OF above. SEEK_RESOURCES enables ResourceSeekingConstraint
	// for every real kind; hunting (ChemotaxisConstraint) reuses the
	// existing PREDATOR_OF field rather than adding a new one.
	bool SEEK_RESOURCES = false;

};

