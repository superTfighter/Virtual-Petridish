#include "PerimeterConstraint.h"

/// <summary>
/// Seems to be working fine.
/// </summary>
PerimeterConstraint::PerimeterConstraint()
{
}

float PerimeterConstraint::deltaH(int sourceI, int targetI, int source_type, int target_type)
{

	if (source_type == target_type) {
		return 0.0f;
	}

	const auto ls = this->model->parameters->LAMBDA_P[this->model->getCellKind(source_type)];
	const auto lt = this->model->parameters->LAMBDA_P[this->model->getCellKind(target_type)];

	if (!(ls > 0) && !(lt > 0)) {
		return 0.0f;
	}

	const auto Ni = this->model->grid.neighi(targetI);

	std::map<int, int> pchange;

	pchange[source_type] = 0; 
	pchange[target_type] = 0;

	for (size_t i = 0; i < Ni.size(); i++)
	{

		const auto nt = this->model->grid.pixti(Ni[i]);

		if (nt != source_type)
		{
			pchange[source_type]++;
		}

		if (nt != target_type)
		{
			pchange[target_type]--;
		}

		if (nt == target_type)
		{
			pchange[nt]++;
		}

		if (nt == source_type)
		{
			pchange[nt]--;
		}

	}

	float r = 0.0f;

	if(ls > 0)
	{
		const auto pt = this->model->parameters->P[this->model->getCellKind(source_type)];
		const auto ps = this->cellPerimeters[source_type];

		const auto hnew = (ps + pchange[source_type]) - pt;
		const auto hold = ps - pt;

		r += ls * ((hnew * hnew) - (hold * hold));
	}

	if(lt > 0)
	{
		const auto pt = this->model->parameters->P[this->model->getCellKind(target_type)];
		const auto ps = this->cellPerimeters[target_type];

		const auto hnew = (ps + pchange[target_type]) - pt;
		const auto hold = ps - pt;

		r += lt * ((hnew * hnew) - (hold * hold));

	}

	return r;
}

void PerimeterConstraint::recomputeCellPerimeters()
{
	cellPerimeters.clear();

	// Iterate model->borderpixels directly rather than through
	// getBorderPixels(): that method gates on the executing/canExecute
	// flags to stay safe when called from a *different* thread (the render
	// thread), but postMCSListener() runs synchronously on the sim thread
	// itself, inside the same monteCarloStep() call that already holds
	// exclusive access -- going through getBorderPixels()'s gate here would
	// spin forever waiting for executing to clear, which this very call is
	// blocking (a self-deadlock).
	const auto neighbourCounts = this->model->perimeterNeighbours();

	for (const auto& index : this->model->borderpixels.elements)
	{
		auto cellID = this->model->grid.pixti(index);

		if (cellID != 0)
		{
			cellPerimeters[cellID] += neighbourCounts[index];
		}
	}
}

void PerimeterConstraint::afterSetModelMethod()
{
	recomputeCellPerimeters();
}

void PerimeterConstraint::postMCSListener()
{
	// Recomputed once per full Monte Carlo step (not per pixel copy): a
	// single-pixel change can shift several neighboring cells' perimeters
	// at once, so a targeted incremental update would be substantially more
	// complex to get right; recomputing here (rather than every accepted
	// pixel copy) keeps the cost at O(border pixels) per step instead of
	// O(border pixels) per copy, which matters once there are many cells.
	recomputeCellPerimeters();
}


