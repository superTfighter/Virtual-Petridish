#pragma once
#include "CellularPotts.h"
#include <vector>

class Statistics
{

public:
	Statistics(CellularPotts* model)
	{
		this->model = model;
	}

	//WE IGNORE THE 0 index
	std::vector<std::vector<std::pair<int, int>>> PixelsByCell();

	//WE IGNORE THE 0 index
	std::vector<std::pair<float, float>> Centoids();

private:

	CellularPotts* model;


};

