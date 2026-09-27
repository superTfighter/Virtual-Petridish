#include "PredationConstraint.h"

float PredationConstraint::deltaH(int sourceI, int targetI, int source_type, int target_type)
{
	if (source_type <= 0 || target_type <= 0)
		return 0.0f;

	auto& predatorOf = this->model->parameters->PREDATOR_OF;

	int sourceKind = this->model->getCellKind(source_type);
	int targetKind = this->model->getCellKind(target_type);

	if (sourceKind < 0 || sourceKind >= (int)predatorOf.size())
		return 0.0f;

	if (predatorOf[sourceKind] == targetKind)
		// Deliberately huge relative to typical adhesion/volume deltaH
		// magnitudes (which scale with LAMBDA_V * deviation^2 and can reach
		// the tens of thousands): predation needs to unconditionally win a
		// predator-vs-prey pixel contest, not just nudge the odds, or two
		// similarly-hungry cells effectively fight to a stalemate via
		// VolumeConstraint alone regardless of this bonus.
		return -1000000.0f;

	return 0.0f;
}
