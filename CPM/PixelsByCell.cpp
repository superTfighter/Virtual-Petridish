#include "PixelsByCell.h"

PixelsByCell::PixelsByCell(CellularPotts* model)
{
	this->model = model;
}

std::vector<std::vector<std::pair<int, int>>> PixelsByCell::getPixelsByCell()
{

	std::vector<std::vector<std::pair<int, int>>> cellPixels;

	auto _pixelArray = this->model->grid._pixelArray;

	int ii = 0;
	int c = 0;

	for (size_t i = 0; i < this->model->grid.size.first; i++)
	{
		
		for (size_t j = 0; j < this->model->grid.size.second; j++)
		{

			if(_pixelArray[ii] > 0)
			{

				std::pair<int, int> p(i, j);
				int id = _pixelArray[ii];

				if (id == 2)
					int a = 0;

				int size = cellPixels.size() - 1;

				if(id < size)
				{
					cellPixels[id].push_back(p);

				}else
				{
					cellPixels.resize(id + 2);
					cellPixels[id].push_back(p);
				}
			}

			ii++;
		}

		c += this->model->grid.x_step;
		ii = c;
	}


	return cellPixels;
	
}
