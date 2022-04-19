#pragma once
#include <vector>
#include "CellularPotts.h"


class PixelsByCell
{

public:

	PixelsByCell(CellularPotts* model);

	CellularPotts* model;

	//1->154=asd,asd
	std::vector<std::vector<std::pair<int,int>>> getPixelsByCell();
};

