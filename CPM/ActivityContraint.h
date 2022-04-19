#pragma once
#include "HamiltonianConstraint.h"
#include <math.h>

class ActivityContraint : public HamiltonianConstraint
{

public:

	ActivityContraint();

	float deltaH(int sourceI, int targetI, int  source_type, int target_type) override;
	
	void afterSetModelMethod() override;
	void postSetpixListener(int i, int t_old, int t_new) override;
	void postMCSListener() override;

	float activityAt(int index);

	std::vector<int> cellpixelsact;

private:
	float activityAtArith(int index);
	float activityAtGeom(int index);

	//TODO:EXCEPTION HANDLING WHEN INDEX IS NOT IN ARRAY
	int pxact(int index);
	

};

