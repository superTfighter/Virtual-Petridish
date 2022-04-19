#include "HamiltonianConstraint.h"

float HamiltonianConstraint::deltaH(int sourceI, int targetI, int source_type, int target_type)
{
    return 0.0f;
}

void HamiltonianConstraint::setModel(CellularPotts* model)
{
    this->model = model;

    afterSetModelMethod();
}

void HamiltonianConstraint::afterSetModelMethod()
{
}

void HamiltonianConstraint::postSetpixListener(int i, int t_old, int t_new)
{
}

void HamiltonianConstraint::postMCSListener()
{
}
