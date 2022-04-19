#include "Grid.h"

Grid::Grid(int xSize, int ySize)
{
	this->size = std::pair<int, int>(xSize, ySize);
	this->_pixelArray = std::vector<int>();

	this->x_bits = 1 + floor(log2(size.first - 1));
	this->y_bits = 1 + floor(log2(size.second - 1));

	this->y_mask = (1 << this->y_bits) - 1;
	this->x_step = 1 << this->y_bits;
	this->y_mask = this->x_step - 1;

	_pixelArray.resize(pointToIndex(size));

	this->middle = std::pair<int,int>(xSize / 2, ySize / 2);

	neighbours.resize(8);
}

int Grid::pointToIndex(std::pair<int, int> point)
{
	if (point.first == -1 && point.second == -1)
		return -1;

	return ( point.first << this->y_bits) + point.second;
}

std::pair<int, int> Grid::indexToPoint(int index)
{
	return std::pair<int, int>(index >> this->y_bits ,index & this->y_mask);
}

void Grid::diffusion(float diffusionCoefficient)
{
	std::vector<int> pixelsBuffer;

	pixelsBuffer.resize(pointToIndex(size));

	for (size_t i = 0; i < size.first; i++)
	{
		for (size_t j = 0; j < size.second; j++)
		{
			int index = i * this->x_step + j;
			pixelsBuffer[index] = this->_pixelArray[index] + diffusionCoefficient * this->laplaciani(index);
		}

	}

	//Swap vectors
	this->_pixelArray.swap(pixelsBuffer);
}

void Grid::setPixel(std::pair<int, int> point, int value)
{
	this->_pixelArray[this->pointToIndex(point)] = value;
}

/// <summary>
/// returns Moore-8 neighbors for pixel coordinate at index
/// </summary>
/// <param name="index"></param>
/// <returns></returns>
std::vector<int> Grid::neighi(int index)
{
	int tl, tm, tr, l, r, bl, bm, br;

	tl = index - 1 - this->x_step;
	tm = index - 1;
	tr = index - 1 + this->x_step;
	l = index - this->x_step;
	r = index + this->x_step;
	bl = index + 1 - this->x_step;
	bm = index + 1;
	br = index + 1 + this->x_step;

	//left border
	if (index < this->size.second) {
	
		tl = -1; l = -1; bl = -1;
	}

	//right border
	if (index >= this->x_step * (this->size.first - 1)) {

		tr = -1; r = -1; br = -1;
	}

	//top border
	if (index % this->x_step == 0) {

		tl = -1; tm = -1; tr = -1;
	}


	//bottom border
	if((index + 1 - this->size.second) % this->x_step == 0)
	{
		bl = -1; bm = -1; br = -1;
	}

	neighbours[0] = tl;
	neighbours[1] = tm;
	neighbours[2] = tr;
	neighbours[3] = l;
	neighbours[4] = r;
	neighbours[5] = bl;
	neighbours[6] = bm;
	neighbours[7] = br;

	return neighbours;
}

int Grid::pixti(int src_i)
{
	if (src_i == -1)
		return -1;
	
	return this->_pixelArray[src_i];
}

void Grid::setpixi(int index, int type)
{
	this->_pixelArray[index] = type;
}

int Grid::laplaciani(int index)
{

	int L = 0, n = 0;
	std::vector<int> neighbours = this->neighNeumanni(index);

	for (size_t i = 0; i < neighbours.size(); i++)
	{
		L += this->_pixelArray[neighbours[i]];

		n++;
	}

	return L - n * this->_pixelArray[index];
}

std::vector<int> Grid::neighNeumanni(int index)
{


	std::vector<int> neighbours;
	neighbours.resize(4);


	int t = index - 1;
	int b = index + 1;
	int l = index - this->x_step;
	int r = index + this->x_step;

	//TOP
	if (index % this->x_step == 0) {
		
		t += this->size.second;
		neighbours[0] = t;
	}
	else {
		neighbours[0] = t;
	}

	//BOTTOM
	if ((index + 1 - this->size.second) % this->x_step == 0) {

		b -= this->size.second;
		neighbours[1] = b;
	}
	else {
		neighbours[1] = b;
	}


	//LEFT
	if (index < this->size.second) {
		
		l += this->size.first * this->x_step;
		neighbours[2] = l;
		
	}
	else {
		neighbours[2] = l;
	}


	//RIGHT
	// right border
	if (index >= this->x_step * (this->size.first - 1)) {
		
		r -= this->size.first * this->x_step;
		neighbours[3] = r;
		
	}
	else {
		neighbours[3] = r;
	}

	return neighbours;
}
