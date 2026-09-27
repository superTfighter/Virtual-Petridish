#pragma once
#include "HamiltonianConstraint.h"
#include <vector>

// Real "hunting": maintains a per-pixel prey-scent field (multi-source BFS
// distance from every pixel belonging to a kind someone is currently
// hunting, per Parameters::PREDATOR_OF), refreshed periodically in
// postMCSListener() (every REFRESH_INTERVAL MCS steps -- a full-grid BFS
// every single step would be wasteful given prey move slowly relative to
// that cadence). deltaH biases a hunter kind's pixel-copy proposals toward
// higher scent (closer to prey).
//
// A local-neighbor-only approximation was considered and rejected: it only
// produces a bias once a hunter is already adjacent to prey, a regime
// PredationConstraint's contact bonus already dominates -- it would not
// look like directed hunting over any real distance, just a random walk
// that happens to finish decisively once already touching.
class ChemotaxisConstraint : public HamiltonianConstraint
{
public:
	ChemotaxisConstraint();

	float deltaH(int sourceI, int targetI, int source_type, int target_type) override;
	void afterSetModelMethod() override;
	void postMCSListener() override;

private:
	void refreshScentField();

	std::vector<float> preyScent;
	int stepsSinceRefresh;

	static constexpr int REFRESH_INTERVAL = 10;
	// preyScent's gradient is a constant ~1 unit per pixel step (see
	// refreshScentField), so LAMBDA directly sets the bias's deltaH
	// magnitude regardless of distance to prey -- needs to be large enough
	// to matter against adhesion/near-target-volume terms (tens to low
	// hundreds), same magnitude-tuning lesson as PredationConstraint's
	// original too-small bonus earlier this session.
	static constexpr float LAMBDA = 500.0f;
};
