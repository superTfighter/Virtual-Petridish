#include "ResourceSeekingConstraint.h"

float ResourceSeekingConstraint::deltaH(int sourceI, int targetI, int source_type, int target_type)
{
	if (!this->model->parameters->SEEK_RESOURCES)
		return 0.0f;

	if (source_type <= 0)
		return 0.0f;

	float sourceResource = this->model->grid.resourceAt(sourceI);
	float targetResource = this->model->grid.resourceAt(targetI);

	// Favorable (negative) when the target pixel has more resource than the
	// source, biasing the cell to grow/move toward the richer pixel.
	return LAMBDA * (sourceResource - targetResource);
}
