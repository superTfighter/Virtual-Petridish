#pragma once
#include "HamiltonianConstraint.h"

class VolumeConstraint : public HamiltonianConstraint
{

public:
	float deltaH(int sourceI, int targetI, int  source_type, int target_type) override;

private:
	float volumeConstraint(float vgain, int cellId);

};

