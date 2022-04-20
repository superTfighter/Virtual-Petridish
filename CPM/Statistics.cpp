#include "Statistics.h"

std::vector<std::vector<std::pair<int, int>>> Statistics::PixelsByCell()
{
	std::vector<int> pixels = this->model->grid._pixelArray;


	std::vector<std::vector<std::pair<int, int>>> pixelsByCell = std::vector<std::vector<std::pair<int, int>>>();

	pixelsByCell.size();

	for (size_t i = 0; i < pixels.size(); i++)
	{

		if (pixels[i] > 0)
		{
			std::pair<int, int> pos = this->model->grid.indexToPoint(i);


			//PUSH TO FIRST VECTOR THE TYPE

			if (pixelsByCell.size() < pixels[i])
			{
				pixelsByCell.resize(pixels[i] + 1);
			}
			//INSERT POSITION TO THAT TYPE

			pixelsByCell[pixels[i]].push_back(pos);

			int a = 0;

		}

	}


	return pixelsByCell;
}

std::vector<std::pair<float, float>> Statistics::Centoids()
{
	std::vector<std::vector<std::pair<int, int>>> pixelsByCell = this->PixelsByCell();

	std::vector<std::pair<float, float>> centoids = std::vector<std::pair<float, float>>();

	for (size_t i = 1; i < pixelsByCell.size(); i++)
	{

		if (pixelsByCell[i].size() > 0)
		{
			int cellID = i;

			std::vector<std::pair<int, int>> pixels = pixelsByCell[i];

			std::pair<float, float> centroid = std::pair<float, float>(0, 0);

			for (size_t dim = 0; dim < 2; dim++)
			{

				float mi = 0.;

				for (size_t j = 0; j < pixels.size(); j++)
				{
					float dx = 0;

					if (dim == 0)
						dx = pixels[j].first - mi;
					else if (dim == 1)
						dx = pixels[j].second - mi;

					mi += dx / (j + 1);
				}

				if (dim == 0)
					centroid.first = mi;
				else if (dim == 1)
					centroid.second = mi;

			}

			if (centoids.size() < cellID)
				centoids.resize(cellID + 1);

			centoids[cellID] = centroid;
		}

	}

	return centoids;
}
