#include "Cell.h"

Cell::Cell(CellularPotts* model, int kindID, int cellID)
{
	this->model = model;
	this->kindID = kindID;
	this->cellID = cellID;
	this->V = 1;
	this->parentID = -1;
}

void Cell::birth(Cell parentID)
{
	this->parentID = parentID.cellID;

	//TODO:FINISH;

	this->V = parentID.V / 2;
	parentID.V = parentID.V / 2;

}
