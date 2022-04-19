#pragma once
#include "HamiltonianConstraint.h"
/**
 * Implements the perimeter constraint of Potts models.
 * A cell's "perimeter" is the number over all its borderpixels of the number of
 * neighbors that do not belong to the cell itself.
 *
 * This constraint is typically used together with.*/
class PerimeterConstraint : public HamiltonianConstraint
{

public:

	PerimeterConstraint();

	float deltaH(int sourceI, int targetI, int  source_type, int target_type) override;
	void afterSetModelMethod() override;
	//void postSetpixListener(int i, int t_old, int t_new);

private:



	//Perimeter size of each cellID
	std::map<int, int> cellPerimeters;



};

