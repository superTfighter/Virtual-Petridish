#include "AdhesionConstraint.h"
#include <vector>
#include <iostream>

float AdhesionConstraint::deltaH(int sourceI, int targetI, int source_type, int target_type)
{
    float result = this->H(targetI, source_type) - this->H(targetI, target_type);
   
    return result;
}

float AdhesionConstraint::H(int i, int tp)
{
    int r = 0;

	std::vector<int> neigbours = this->model->grid.neighi(i);

    for (auto& elem : neigbours)
    {
        if(elem != -1)
        {
            int tn = this->model->grid.pixti(elem);

            if (tn != tp)
                r += this->J(tn, tp);
        }
    }

    return r;
}

float AdhesionConstraint::J(int t1, int t2)
{
    int idx1 = this->model->parameters->stateIndex(this->model->getCellKind(t1), this->model->getCellState(t1));
    int idx2 = this->model->parameters->stateIndex(this->model->getCellKind(t2), this->model->getCellState(t2));

    std::vector<int> J = this->model->parameters->J[idx1];

    return J[idx2];
}
