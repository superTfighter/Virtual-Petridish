#pragma once
#include "CellularPotts.h"
#include "Statistics.h"
#include <math.h>

class GridManadger
{

public:

	GridManadger(CellularPotts* model);
	CellularPotts* model;

	void divideCell(int cellID);


private:



};

