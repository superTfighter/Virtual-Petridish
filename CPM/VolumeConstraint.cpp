#include "VolumeConstraint.h"

float VolumeConstraint::deltaH(int sourceI, int targetI, int source_type, int target_type)
{
    float deltaH = 0;

    deltaH = this->volumeConstraint(1, source_type) - this->volumeConstraint(0, source_type);
    deltaH += this->volumeConstraint(-1,target_type) - this->volumeConstraint(0,target_type);

    return deltaH;
}

float VolumeConstraint::volumeConstraint(float vgain, int cellId)
{
    const float l = this->model->parameters->LAMBDA_V[this->model->getCellKind(cellId)];

    if (cellId == 0 || l == 0)
        return 0;

    float volume = this->model->getCellVolume(cellId);

    const float vdiff = this->model->parameters->V[this->model->getCellKind(cellId)] - (volume + vgain);

    return l * vdiff * vdiff;
}
