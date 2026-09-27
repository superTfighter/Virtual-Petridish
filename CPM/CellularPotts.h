#pragma once
#include "Parameters.h"
#include "Grid.h"
#include "HamiltonianConstraint.h"
#include "DiceSet.h"
#include "Cell.h"
#include <chrono>
#include <thread>
#include "GridManadger.h"
#include <random>
#include <atomic>




class HamiltonianConstraint;
class Cell;


class CellularPotts
{
public:
	
	CellularPotts();

	CellularPotts(std::pair<int, int> gridSize, Parameters *parameters);

	// Explicit copy ctor/assignment needed because std::atomic<bool> members
	// (executing/canExecute/stopRequested/makingANewCellID/settingAPixel) are
	// not copyable by default; Simulation::setupSimulation() relies on
	// `model = CellularPotts(...)` to reset the simulation, so this must stay
	// assignable.
	CellularPotts(const CellularPotts& other);
	CellularPotts& operator=(const CellularPotts& other);

	void init(std::pair<int, int> gridSize, Parameters* parameters);

	void addConstraint(HamiltonianConstraint* c);
	std::vector<HamiltonianConstraint* > getAllContraints();

	void monteCarloStep();
	void monteCarloParallel();

	int makeNewCellID(int kind);

	void setCellKind(int typeID, int kind);
	int getCellKind(int typeID);

	// A cell's substate (default 0 -- "base"), independent of its kind.
	// Parameters::stateIndex(kind, state) combines the two into a single
	// flat index for constraints that want per-(kind,state) parameter
	// arrays instead of per-kind ones.
	void setCellState(int typeID, int state);
	int getCellState(int typeID);

	void updateBorderNearAri(int index, int old_type, int new_type);

	void setPixelI(int cellId, int sourceType);
	void setPixel(std::pair<int, int> point, int sourceType);
	bool addCellAt(std::pair<int, int> point, int kind);

	int getCellVolume(int cellId);
	void updateCellVolumes();

	std::vector<int> perimeterNeighbours();
	std::vector<std::pair<int, int>> getBorderPixels();
	
	std::vector<Cell> cells;
	Grid grid;
	Parameters *parameters;
	DiceSet borderpixels;
	int simTime;

	void birth(int childID,int parentID);

	std::atomic<bool> executing;
	std::atomic<bool> canExecute;
	std::atomic<bool> stopRequested;
	std::atomic<int> stepDelayMs;
	bool cellDivision;

	std::vector<void (*)()> postMCstepFunctions;

	void addPostMCstepFunction(void (*function)());

	unsigned char* getRenderImage();
	unsigned char* getRenderImage(std::vector<int>& activityVector);

	int getCellCount();
	float getAreaCoveredByCells();

private:
	int last_cell_id;

	float deltaH(int sourceIndex,int targetIndex,int sourceType,int targetType);
	bool docopy(float deltaH);

	void parallelImageCalc(unsigned char* image, int startIndex, int endIndex,int i);

	std::vector<int> cellVolume;
	std::vector<int> cellTypeToKind;
	std::vector<int> cellTypeToState;
	std::vector<int> _neighbours;

	std::vector<HamiltonianConstraint*> contraints;

	std::atomic<bool> makingANewCellID;
	std::atomic<bool> settingAPixel;

	char* previousImage;

	// Owns the Parameters instance the default ctor points `parameters` at,
	// so it isn't a dangling pointer to a stack temporary that's already
	// been destroyed by the time anyone dereferences it.
	Parameters defaultParameters;

#ifdef CPM_ENABLE_TEST_HOOKS
	// docopy() implements the Metropolis acceptance criterion and has no
	// other public surface to verify it through (tests/test_docopy.cpp
	// regression-tests the exact bug fixed in feature_list.txt item 1).
	// Gated behind a build-time macro (only defined for the test suite, see
	// tests/CMakeLists.txt) so normal builds are completely unaffected.
	friend bool testAccessDocopy(CellularPotts& model, float deltaH);
#endif
};

#ifdef CPM_ENABLE_TEST_HOOKS
inline bool testAccessDocopy(CellularPotts& model, float deltaH)
{
	return model.docopy(deltaH);
}
#endif

