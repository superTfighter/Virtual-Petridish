#include "GridManadger.h"
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

void GridManadger::divideCell(int cellID)
{
	Statistics stats = Statistics(this->model);

	auto centroids = stats.Centoids()[cellID];
	auto pixelsByCell = stats.PixelsByCell()[cellID];

	double bxx = 0, bxy = 0, byy = 0, cx, cy, x2, y2, side, T,D, x0, y0, x1, y1, L2;

	for (size_t j = 0;  j < pixelsByCell.size();  j++)
	{
		cx = pixelsByCell[j].first - centroids.first;
		cy = pixelsByCell[j].second - centroids.second;


		bxx += cx * cx;
		bxy += cx * cy;
		byy += cy * cy;

	}

	if(bxy == 0)
	{
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

	auto nid = this->model->makeNewCellID(this->model->getCellKind(cellID));

	for (size_t j = 0; j < pixelsByCell.size(); j++)
	{
		x2 = pixelsByCell[j].first - centroids.first;
		y2 = pixelsByCell[j].second - centroids.second;

		side = (x1 - x0) * (y2 - y0) - (x2 - x0) * (y1 - y0);

		if(side > 0)
		{
			model->setPixel(pixelsByCell[j], nid);
		}

	}

}
