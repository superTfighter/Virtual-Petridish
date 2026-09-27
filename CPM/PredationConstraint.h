#pragma once
#include "HamiltonianConstraint.h"

// A predator-kind cell is strongly energetically favored to invade pixels
// currently held by its configured prey kind (Parameters::PREDATOR_OF), on
// top of whatever AdhesionConstraint already contributes. A prey cell that
// loses all its pixels this way just dies via the same generic zero-volume
// handling every other cell death already goes through
// (CellularPotts::setPixelI) -- no separate death/removal machinery needed,
// predation is just a strong bias on the existing pixel-copy contest.
class PredationConstraint : public HamiltonianConstraint
{
public:
	float deltaH(int sourceI, int targetI, int source_type, int target_type) override;
};
