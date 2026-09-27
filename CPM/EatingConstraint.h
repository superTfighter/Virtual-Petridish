#pragma once
#include "HamiltonianConstraint.h"

// Decrements the grid's nutrient/resource level as a cell moves onto a
// pixel, at a per-cell-kind rate (Parameters::CONSUMPTION_RATE). Purely a
// bookkeeping constraint -- no deltaH term, so it doesn't bias the
// simulation toward/away from resource-rich pixels, it just tracks
// consumption as cells wander over the grid.
class EatingConstraint : public HamiltonianConstraint
{

public:
	void postSetpixListener(int i, int t_old, int t_new) override;
};
