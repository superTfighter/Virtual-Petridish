#include "ActivityContraint.h"

ActivityContraint::ActivityContraint()
{
	this->cellpixelsact = std::vector<int>();
}

float ActivityContraint::deltaH(int sourceI, int targetI, int source_type, int target_type)
{

	if (this->model->parameters->ACT_MEAN == "false")
		throw "Activity contraint used when not active";

	float deltaH = 0;
	float  maxAct = 0;
	float lambdaAct = 0;

	if(source_type != 0)
	{
		maxAct = this->model->parameters->MAX_ACT[source_type];
		lambdaAct = this->model->parameters->LAMBDA_ACT[source_type];
	}else
	{
		maxAct = this->model->parameters->MAX_ACT[target_type];
		lambdaAct = this->model->parameters->LAMBDA_ACT[target_type];
	}

	if(maxAct == 0 || lambdaAct == 0)
	{
		return 0;
	}

	deltaH += lambdaAct * (this->activityAt(targetI) - this->activityAt(sourceI)) / maxAct;

	return deltaH;
}

void ActivityContraint::afterSetModelMethod()
{
	this->cellpixelsact.resize(this->model->grid._pixelArray.size());

}

void ActivityContraint::postSetpixListener(int i, int t_old, int t_new)
{
	this->cellpixelsact[i] = this->model->parameters->MAX_ACT[t_new];
}

void ActivityContraint::postMCSListener()
{

	for (size_t i = 0; i < this->cellpixelsact.size(); i++)
	{

		if (this->cellpixelsact[i] > 0)
		{
			int value = this->cellpixelsact[i];
			this->cellpixelsact[i]--;
		}
	}

}

float ActivityContraint::activityAt(int index)
{
	if (this->model->parameters->ACT_MEAN == "false")
		throw "Activity contraint used when not active";

	if (this->model->parameters->ACT_MEAN == "geometric")
		return activityAtGeom(index);
	else if (this->model->parameters->ACT_MEAN == "arithmetic")
		return activityAtArith(index);
}

float ActivityContraint::activityAtArith(int index)
{
	const int type = this->model->grid.pixti(index);

	if(type <= 0)
		return 0;

	const std::vector<int> Neighbours = this->model->grid.neighi(index);

	int r = this->pxact(index);	
	int numberNeigbours = 1;

	for (size_t i = 0; i < Neighbours.size(); i++)
	{
		const int typeNeigbour = this->model->grid.pixti(Neighbours[i]);


		if(typeNeigbour == type)
		{
			r += this->pxact(Neighbours[i]);

			numberNeigbours++;
		}

	}


	return r / numberNeigbours;
}

float ActivityContraint::activityAtGeom(int index)
{
	const int type = this->model->grid.pixti(index);

	if (type <= 0)
		return 0;

	const std::vector<int> Neighbours = this->model->grid.neighi(index);

	int r = this->pxact(index);
	int numberNeigbours = 1;

	for (size_t i = 0; i < Neighbours.size(); i++)
	{
		const int typeNeigbour = this->model->grid.pixti(Neighbours[i]);

		if (typeNeigbour == type)
		{
			if (this->pxact(Neighbours[i] == 0))
				return 0;

			r *= this->pxact(Neighbours[i]);

			numberNeigbours++;
		}
	}

	return std::pow(r, 1 / numberNeigbours);
}

int ActivityContraint::pxact(int index)
{
	try
	{
		return cellpixelsact.at(index);
	}
	catch (const std::out_of_range& oor) {
		return 0;
	}
}
