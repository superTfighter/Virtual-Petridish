#include "PersistenceConstraint.h"

float PersistenceConstraint::deltaH(int sourceI, int targetI, int source_type, int target_type)
{
	if (source_type == 0 || !celldirections.count(source_type))
		return 0.0f;

	auto b = this->celldirections[source_type];
	auto p1 = this->model->grid.indexToPoint(sourceI);
	auto p2 = this->model->grid.indexToPoint(targetI);

	std::map<int, int> a;

	a[0] = p2.first - p1.first;
	a[1] = p2.second - p2.second;

	float dp = 0.0f;
	dp += a[0] * b.first;
	dp += a[1] * b.second;

	return -dp;
}

void PersistenceConstraint::afterSetModelMethod()
{
	this->halfsize.resize(2);

	halfsize[0] = this->model->grid.size.first / 2;
	halfsize[1] = this->model->grid.size.second / 2;
}

//TODO:DO THIS
void PersistenceConstraint::postMCSListener()
{


}
