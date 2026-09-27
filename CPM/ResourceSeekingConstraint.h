#pragma once
#include "HamiltonianConstraint.h"

// When Parameters::SEEK_RESOURCES is true, biases every real kind's
// pixel-copy proposals toward moving into higher-resource pixels. Reads
// CellularPotts::grid's resource field directly -- no new field to
// maintain, unlike ChemotaxisConstraint's prey-scent field.
class ResourceSeekingConstraint : public HamiltonianConstraint
{
public:
	float deltaH(int sourceI, int targetI, int source_type, int target_type) override;

private:
	// VolumeConstraint's growth-phase deltaH can reach the thousands (a
	// cell far below its target volume), which swamped an initial LAMBDA=2
	// -- the resulting "drift" was indistinguishable from noise in testing
	// (a control run with seeking disabled drifted just as much, sometimes
	// more). Needs to be large enough to matter against that, same lesson
	// as ChemotaxisConstraint::LAMBDA and PredationConstraint's bonus.
	static constexpr float LAMBDA = 100.0f;
};
