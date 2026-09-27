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

	// Nutrient/resource level per grid position (same padded stride as
	// _pixelArray), consumed by EatingConstraint as cells move over it.
	// Named so the renderer can normalize its resource-level tint against
	// the same starting value every pixel is filled with (Grid.cpp ctor).
	static constexpr float INITIAL_RESOURCE = 100.0f;
	std::vector<float> _resourceArray;
	float resourceAt(int index);
	void consumeResourceAt(int index, float amount);

	int x_step;
	int x_bits;
	int y_bits;
	int y_mask;


	std::vector<int> actuallyRenderPixelArray;

	//Return -1 for neighbours outside of border
	std::vector<int> neighi(int index);
	int pixti(int  src_i);

	void setpixi(int index, int type);


private:
	int laplaciani(int index);

	//Von Neumann neighborhood -- only 4 top,bottom,left,right
	std::vector<int> neighNeumanni(int index);

};

