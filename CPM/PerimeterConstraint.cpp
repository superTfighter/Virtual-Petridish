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

		r += ls * ((hnew * hnew) - (hold * hold));

	}

	return r;
}

void PerimeterConstraint::afterSetModelMethod()
{
	auto borderPixels = this->model->getBorderPixels();

	for (size_t i = 0; i < borderPixels.size(); i++)
	{
		auto cellID = this->model->grid.pixti(this->model->grid.pointToIndex(borderPixels[i]));

		if (cellID != -1)
		{

			if (cellPerimeters.count(cellID)) 
			{
				const auto index = this->model->grid.pointToIndex(borderPixels[i]);

				cellPerimeters[cellID] += this->model->perimeterNeighbours()[index];
			}
			else 
			{
				cellPerimeters[cellID] = 0;

			};
		}

	}

}


