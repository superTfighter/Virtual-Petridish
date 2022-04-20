#pragma once
#include "HamiltonianConstraint.h"
#include "Statistics.h"

class PersistenceConstraint : public HamiltonianConstraint
{

public:
	float deltaH(int sourceI, int targetI, int  source_type, int target_type) override;
	
	void afterSetModelMethod() override;
	//void postSetpixListener(int i, int t_old, int t_new);
	void postMCSListener();

private:

	std::vector<int> halfsize;

	std::vector<std::pair<float, float>> cellcentroidlists;
	std::map<int, std::pair<int,int>> celldirections;



};

