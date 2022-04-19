#pragma once
#include "HamiltonianConstraint.h"

class PersistenceConstraint : public HamiltonianConstraint
{

public:
	float deltaH(int sourceI, int targetI, int  source_type, int target_type) override;
	
	void afterSetModelMethod() override;
	//void postSetpixListener(int i, int t_old, int t_new);
	void postMCSListener();

private:

	std::vector<int> halfsize;

	std::map<int, int> cellcentroidlists;
	std::map<int, std::pair<int,int>> celldirections;



};

