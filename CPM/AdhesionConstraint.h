#pragma once
#include "HamiltonianConstraint.h"


class AdhesionConstraint : public HamiltonianConstraint
{

public:
	float deltaH(int sourceI, int targetI, int  source_type, int target_type) override;

private:

	float H(int i ,int tp);
	float J(int t1, int t2);

};

