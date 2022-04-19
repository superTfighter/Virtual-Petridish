#include "GridManadger.h"
#include "PixelsByCell.h"
#include "Centroids.h"
#include <thread>

GridManadger::GridManadger(CellularPotts* model)
{
	this->model = model;
}

int GridManadger::seedCell(int kind, int maxAttempts)
{
	//std::pair<int, int> position = this->model.grid.middle;


	////TODO:FIX, SEARCH FOR 0 POSITION AROUND THE MIDDLE!!
	///*while (this->model.grid.pointToIndex(middle) != 0 && maxAttempts-- > 0)
	//{
	//	

	//}*/

	//int newID = this->model.makeNewCellID(kind);
	//this->model.grid.setPixel(position, newID);

	//return newID;

	return -1;
}

void GridManadger::divideCell(Cell cell)
{
	auto cp = PixelsByCell(this->model).getPixelsByCell()[cell.cellID];
	auto com = Centroids(this->model).getCentroids()[cell.cellID];

	double bxx, bxy, byy, cx, cy, x2, y2, side, T, D, x0, y0, x1, y1, L2;
	bxx = bxy = byy = cx = cy = x2 = y2 = side = T = D = x0 = y0 = x1 = y1 = L2 = 0;

	for (size_t i = 0; i < cp.size(); i++)
	{
		cx = cp[i].first - com[0];
		cy = cp[i].second - com[1];

		bxx += cx * cx;
		bxy += cx * cy;
		byy += cy * cy;
	}

	if (bxy == 0) {

		x0 = 0;
		y0 = 0;
		x1 = 1;
		y1 = 0;
	}
	else
	{
		T = bxx + byy;
		D = bxx * byy - bxy * bxy;

		L2 = T / 2 - std::sqrt(T * T / 4 - D);
		x0 = 0;
		y0 = 0;
		x1 = L2 - byy;
		y1 = bxy;

	}

	int newID = model->makeNewCellID(cell.kindID);
	model->birth(newID, cell.cellID);

	for (size_t i = 0; i < cp.size(); i++)
	{
		x2 = cp[i].first - com[0];
		y2 = cp[i].second - com[1];

		double side = (x1 - x0) * (y2 - y0) - (x2 - x0) * (y1 - y0);

		if (side > 0) {

			model->setPixel(cp[i], newID);
		}

	}

	//update cellvolumes
	this->model->updateCellVolumes();

}
