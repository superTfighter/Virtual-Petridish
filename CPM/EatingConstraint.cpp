#include "EatingConstraint.h"

void EatingConstraint::postSetpixListener(int i, int t_old, int t_new)
{
	if (t_new == 0)
		return;

	int kind = this->model->getCellKind(t_new);
	auto& rate = this->model->parameters->CONSUMPTION_RATE;

	if (kind < 0 || kind >= (int)rate.size())
		return;

	this->model->grid.consumeResourceAt(i, rate[kind]);
}
