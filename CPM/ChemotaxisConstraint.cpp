#include "ChemotaxisConstraint.h"
#include <queue>
#include <algorithm>

ChemotaxisConstraint::ChemotaxisConstraint()
{
	stepsSinceRefresh = 0;
}

void ChemotaxisConstraint::afterSetModelMethod()
{
	preyScent.assign(this->model->grid._pixelArray.size(), 0.0f);
	stepsSinceRefresh = 0;
	refreshScentField();
}

void ChemotaxisConstraint::postMCSListener()
{
	stepsSinceRefresh++;

	if (stepsSinceRefresh >= REFRESH_INTERVAL)
	{
		refreshScentField();
		stepsSinceRefresh = 0;
	}
}

// Runs on the sim thread only, synchronously within monteCarloStep()'s own
// call chain (same as PerimeterConstraint::postMCSListener()) -- safe to
// access model->grid directly, no cross-thread gating needed.
void ChemotaxisConstraint::refreshScentField()
{
	auto& predatorOf = this->model->parameters->PREDATOR_OF;

	bool anyHunting = false;
	for (int p : predatorOf)
	{
		if (p != 0) { anyHunting = true; break; }
	}

	std::fill(preyScent.begin(), preyScent.end(), 0.0f);

	if (!anyHunting)
		return;

	// Multi-source BFS from every pixel belonging to a currently-hunted prey
	// kind. preyScent = -distance, NOT a decaying 1/(1+distance) field: the
	// latter's gradient shrinks quadratically with distance (e.g. ~0.0002
	// at 75px out), making deltaH's bias vanish long before a hunter could
	// ever close real hunting range -- confirmed by an earlier failed test
	// where hunting made no measurable progress in 4s at 150px separation.
	// Using raw negative distance gives a *constant* ~1-unit-per-pixel-step
	// gradient everywhere, so the bias magnitude doesn't depend on how far
	// away the prey currently is.
	std::vector<int> distance(this->model->grid._pixelArray.size(), -1);
	std::queue<int> frontier;

	for (size_t i = 0; i < this->model->grid._pixelArray.size(); i++)
	{
		int cellId = this->model->grid._pixelArray[i];

		if (cellId <= 0)
			continue;

		int kind = this->model->getCellKind(cellId);

		bool isPrey = false;
		for (int p : predatorOf)
		{
			if (p == kind) { isPrey = true; break; }
		}

		if (isPrey)
		{
			distance[i] = 0;
			preyScent[i] = 0.0f;
			frontier.push((int)i);
		}
	}

	while (!frontier.empty())
	{
		int idx = frontier.front();
		frontier.pop();

		for (int n : this->model->grid.neighi(idx))
		{
			if (n >= 0 && distance[n] == -1)
			{
				distance[n] = distance[idx] + 1;
				preyScent[n] = -(float)distance[n];
				frontier.push(n);
			}
		}
	}
}

float ChemotaxisConstraint::deltaH(int sourceI, int targetI, int source_type, int target_type)
{
	if (source_type <= 0)
		return 0.0f;

	int kind = this->model->getCellKind(source_type);
	auto& predatorOf = this->model->parameters->PREDATOR_OF;

	if (kind < 0 || kind >= (int)predatorOf.size() || predatorOf[kind] == 0)
		return 0.0f;

	// Favorable (negative) when the target pixel is closer to prey (higher
	// scent) than the source, biasing the hunter to move toward its prey.
	return LAMBDA * (preyScent[sourceI] - preyScent[targetI]);
}
