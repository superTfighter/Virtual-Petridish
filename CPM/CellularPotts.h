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




class HamiltonianConstraint;
class Cell;


class CellularPotts
{
public:
	
	CellularPotts();

	CellularPotts(std::pair<int, int> gridSize, Parameters *parameters);

	void init(std::pair<int, int> gridSize, Parameters* parameters);

	void addConstraint(HamiltonianConstraint* c);
	std::vector<HamiltonianConstraint* > getAllContraints();

	void monteCarloStep();
	void monteCarloParallel();

	int makeNewCellID(int kind);

	void setCellKind(int typeID, int kind);
	int getCellKind(int typeID);

	void updateBorderNearAri(int index, int old_type, int new_type);

	void setPixelI(int cellId, int sourceType);
	void setPixel(std::pair<int, int> point, int sourceType);

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

	bool executing;
	bool canExecute;
	bool cellDivision;

	std::vector<void (*)()> postMCstepFunctions;

	void addPostMCstepFunction(void (*function)());

	unsigned char* getRenderImage();
	unsigned char* getRenderImage(std::vector<int>& activityVector);

	int getCellCount();
	int getCellTypeCount();
	float getAreaCoveredByCells();

private:
	int number_of_cells;
	int last_cell_id;

	float deltaH(int sourceIndex,int targetIndex,int sourceType,int targetType);
	bool docopy(float deltaH);

	void parallelImageCalc(unsigned char* image, int startIndex, int endIndex,int i);

	std::vector<int> cellVolume;
	std::vector<int> cellTypeToKind;
	std::vector<int> _neighbours;

	std::vector<HamiltonianConstraint*> contraints;

	bool makingANewCellID;
	bool settingAPixel;

	char* previousImage;
};

