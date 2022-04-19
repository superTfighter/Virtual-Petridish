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
            int tn = this->model->getCellKind(this->model->grid.pixti(elem));

            if (tn != tp)
                r += this->J(tn, tp);
        }
    }

    return r;
}

float AdhesionConstraint::J(int t1, int t2)
{
    if (t1 == 1 && t2 == 2)
        int a = 0;


    std::vector<int> J = this->model->parameters->J[this->model->getCellKind(t1)];

    return J[this->model->getCellKind(t2)];
}
