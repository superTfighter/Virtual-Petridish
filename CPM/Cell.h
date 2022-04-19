#pragma once
#include "CellularPotts.h"

class CellularPotts;

class Cell
{

public:

	Cell(CellularPotts* model, int kindID,int cellID);

	CellularPotts* model;

	int kindID;
	int cellID;
	int parentID;

	float V;
	std::vector<Cell> products;

	void birth(Cell parentID);

private:

};

