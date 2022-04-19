#pragma once
#include "CellularPotts.h"


class CellularPotts;

class HamiltonianConstraint
{
public:

	void setModel(CellularPotts* model);
	
	virtual float deltaH(int sourceI, int targetI,int  source_type,int target_type);
	virtual void afterSetModelMethod();
	virtual void postSetpixListener(int i, int t_old, int t_new);
	virtual void postMCSListener();

protected:
	CellularPotts* model;

private:
	

};

