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

};

