#include "Centroids.h"


Centroids::Centroids(CellularPotts* model)
{
	this->model = model;
}

std::vector<std::vector<int>> Centroids::getCentroids()
{

	std::vector<std::vector<std::pair<int, int>>> cp = PixelsByCell(this->model).getPixelsByCell();
	std::vector<std::vector<int>> centroids = std::vector<std::vector<int>>();

	auto cells = model->cells;

	for (auto cell : cells)
	{
		auto id = cell.cellID;

		int size = centroids.size() - 1;

		if (id < size)
		{
			centroids[id] = this->computeCentroid(id, cp);
		}
		else
		{
			centroids.resize((int)(id + 2));
			centroids[id] = this->computeCentroid(id, cp);
		}
	}

	return centroids;
}

std::vector<int> Centroids::computeCentroid(int cellId, std::vector<std::vector<std::pair<int, int>>> cellPixels)
{
	std::vector<std::pair<int, int>> pixels;
	std::vector<int> cvec;

	pixels = cellPixels[cellId];

	cvec = std::vector<int>();

	cvec.resize(2);

	for (size_t i = 0; i < 2; i++)
	{
		float mi = .0;

		for (size_t j = 0; j < pixels.size(); j++)
		{
			float dx = 0;

			if (i == 0)
			{
				dx = pixels[j].first - mi;

			}
			else
			{

				dx = pixels[j].second - mi;

			}

			mi += dx / (j + 1);
		}

		cvec[i] = mi;
	}

	return cvec;
}
