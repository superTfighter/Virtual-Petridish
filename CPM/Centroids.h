#pragma once
#include "CellularPotts.h"
#include "PixelsByCell.h"

class Centroids
{

public:

	Centroids(CellularPotts* model);

	CellularPotts* model;

	std::vector<std::vector<int>> getCentroids();

private:

	std::vector<int> computeCentroid(int cellId, std::vector<std::vector<std::pair<int, int>>> cellPixels);

};

