#pragma once
#include "CellularPotts.h"

class GridManadger
{

public:

	GridManadger(CellularPotts* model);
	CellularPotts* model;

	//void killCell(int cellID);
	int seedCell(int kind, int maxAttempts = 10000);
	//void seedCellAt(int kind, std::pair<int, int> position);
	void divideCell(Cell cell);


private:



};

