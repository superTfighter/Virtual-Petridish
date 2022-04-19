#pragma once
#include <utility>
#include <math.h>
#include <iostream>
#include <vector>

class Grid
{
public:

	Grid(int xSize, int ySize);

	int pointToIndex(std::pair<int, int> point);
	std::pair<int, int> indexToPoint(int index);

	void diffusion(float diffusionCoefficient);
	void setPixel(std::pair<int, int> point, int value);


	std::pair<int, int> size;
	std::pair<int, int> middle;

	std::vector<int> _pixelArray;
	int x_step;

	//Return -1 for neighbours outside of border
	std::vector<int> neighi(int index);
	int pixti(int  src_i);

	void setpixi(int index, int type);


private:
	int x_bits;
	int y_bits;
	int y_mask;
	

	int laplaciani(int index);

	//Von Neumann neighborhood -- only 4 top,bottom,left,right
	std::vector<int> neighNeumanni(int index);

	std::vector<int> neighbours;

};

