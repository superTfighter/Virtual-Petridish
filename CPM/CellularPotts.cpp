#include "CellularPotts.h"
#include "PixelsByCell.h"
#include <stdexcept>



CellularPotts::CellularPotts() : grid(0, 0)
{
	init(std::pair<int, int>{0, 0}, &defaultParameters);
}

//TODO:OPTIMIZE THESE!!
CellularPotts::CellularPotts(std::pair<int, int> gridSize, Parameters* parameters) : grid(gridSize.first, gridSize.second), parameters(parameters)
{
	this->init(gridSize, parameters);
}

CellularPotts::CellularPotts(const CellularPotts& other) :
	cells(other.cells),
	grid(other.grid),
	parameters(other.parameters),
	borderpixels(other.borderpixels),
	simTime(other.simTime),
	executing(other.executing.load()),
	canExecute(other.canExecute.load()),
	stopRequested(other.stopRequested.load()),
	stepDelayMs(other.stepDelayMs.load()),
	cellDivision(other.cellDivision),
	postMCstepFunctions(other.postMCstepFunctions),
	last_cell_id(other.last_cell_id),
	cellVolume(other.cellVolume),
	cellTypeToKind(other.cellTypeToKind),
	cellTypeToState(other.cellTypeToState),
	_neighbours(other._neighbours),
	contraints(other.contraints),
	makingANewCellID(other.makingANewCellID.load()),
	settingAPixel(other.settingAPixel.load()),
	previousImage(other.previousImage),
	defaultParameters(other.defaultParameters)
{
}

CellularPotts& CellularPotts::operator=(const CellularPotts& other)
{
	if (this == &other)
		return *this;

	cells = other.cells;
	grid = other.grid;
	parameters = other.parameters;
	borderpixels = other.borderpixels;
	simTime = other.simTime;
	executing = other.executing.load();
	canExecute = other.canExecute.load();
	stopRequested = other.stopRequested.load();
	stepDelayMs = other.stepDelayMs.load();
	cellDivision = other.cellDivision;
	postMCstepFunctions = other.postMCstepFunctions;
	last_cell_id = other.last_cell_id;
	cellVolume = other.cellVolume;
	cellTypeToKind = other.cellTypeToKind;
	cellTypeToState = other.cellTypeToState;
	_neighbours = other._neighbours;
	contraints = other.contraints;
	makingANewCellID = other.makingANewCellID.load();
	settingAPixel = other.settingAPixel.load();
	previousImage = other.previousImage;
	defaultParameters = other.defaultParameters;

	return *this;
}

void CellularPotts::init(std::pair<int, int> gridSize, Parameters* parameters)
{
	previousImage = nullptr;
	makingANewCellID = false;
	settingAPixel = false;
	executing = false;
	stopRequested = false;
	stepDelayMs = 0;
	cellDivision = false;


	this->grid = Grid(gridSize.first, gridSize.second);
	this->parameters = parameters;


	this->cells = std::vector<Cell>();

	this->cellVolume = std::vector<int>();
	cellVolume.resize(1);

	this->cellTypeToKind = std::vector<int>();
	cellTypeToKind.resize(1);

	this->cellTypeToState = std::vector<int>();
	cellTypeToState.resize(1);

	this->_neighbours = std::vector<int>();
	_neighbours.resize(gridSize.first * gridSize.second * 8);

	this->simTime = 0;


	for (size_t i = 0; i < grid.size.first; i++)
	{
		for (size_t j = 0; j < grid.size.second; j++)
		{
			setPixel(std::pair<int, int>(i, j), 0);
		}
	}

	this->last_cell_id = 0;
}

void CellularPotts::addConstraint(HamiltonianConstraint* c)
{
	c->setModel(this);
	this->contraints.push_back(c);
}

std::vector<HamiltonianConstraint*> CellularPotts::getAllContraints()
{
	return this->contraints;
}

//TODO:REFACTOR THIS
void CellularPotts::monteCarloStep()
{
	if (this->borderpixels.length == 0)
		return;

	float delta_t = 0.0f;

	while (delta_t < 1.0f)
	{
		delta_t += 1. / (this->borderpixels.length);

		auto tgt_i = this->borderpixels.sample();
		auto Ni = this->grid.neighi(tgt_i);

		bool gotNeigbour = false;

		int src_i = -1;

		while (!gotNeigbour)
		{

			int r = rand() % ((Ni.size() - 1) - 0 + 1) + 0;

			src_i = Ni[r];

			if (0 <= src_i)
				gotNeigbour = true;
		}

		auto src_type = this->grid.pixti(src_i);
		auto tgt_type = this->grid.pixti(tgt_i);

		if (tgt_type != src_type)
		{
			auto hamiltonian = this->deltaH(src_i, tgt_i, src_type, tgt_type);

			if (this->docopy(hamiltonian))
				this->setPixelI(tgt_i, src_type);
		}
	}

	for (auto& constraint : this->contraints)
	{
		constraint->postMCSListener();
	}

	for (size_t i = 0; i < postMCstepFunctions.size(); i++)
	{
		postMCstepFunctions[i]();
	}

	if(cellDivision)
	{
		GridManadger manadger = GridManadger(this);

		std::mt19937 generator = std::mt19937(time(NULL));
		std::uniform_real_distribution<double> dis(0, 1);

		for (size_t i = 1; i < this->cellTypeToKind.size(); i++)
		{
			int cellID = i;

			int cellVolume = this->getCellVolume(cellID);
			int cellMaxVolume = this->parameters->V[this->getCellKind(cellID)];

			double random = dis(generator);

			//random = 0;

			if(cellVolume > cellMaxVolume*0.95 && random < 0.1)
			{
				manadger.divideCell(cellID);
			}


		}

	}

	this->updateCellVolumes();

	this->simTime++;
}

void CellularPotts::monteCarloParallel()
{
	canExecute = true;
	stopRequested = false;

	while (!stopRequested)
	{
		executing = true;

		if (canExecute)
			monteCarloStep();

		executing = false;

		int delay = stepDelayMs.load();

		if (delay > 0)
			std::this_thread::sleep_for(std::chrono::milliseconds(delay));
	}
}

int CellularPotts::makeNewCellID(int kind)
{

	if (!makingANewCellID) {

		makingANewCellID = true;

		int newID = ++this->last_cell_id;

		try {
			cellVolume.insert(cellVolume.begin() + newID, kind);
		}
		catch (std::out_of_range& err) {

			cellVolume.resize(cellVolume.size() * 2);
			cellVolume.insert(cellVolume.begin() + newID, kind);
		}

		this->setCellKind(newID, kind);
		this->setCellState(newID, 0);
		cells.push_back(Cell(this, kind, newID));

		makingANewCellID = false;

		return newID;
	}else
	{

		while (makingANewCellID)
		{
			std::this_thread::sleep_for(std::chrono::microseconds(1));
		}

		return makeNewCellID(kind);
	}

	
}

void CellularPotts::setCellKind(int typeID, int kind)
{

	try {
		cellTypeToKind.insert(cellTypeToKind.begin() + typeID, kind);
	}
	catch (std::out_of_range& err) {

		cellTypeToKind.resize(cellTypeToKind.size() * 2);
		cellTypeToKind.insert(cellTypeToKind.begin() + typeID, kind);
	}
}

int CellularPotts::getCellKind(int typeID)
{
	return this->cellTypeToKind[typeID];
}

void CellularPotts::setCellState(int typeID, int state)
{

	try {
		cellTypeToState.insert(cellTypeToState.begin() + typeID, state);
	}
	catch (std::out_of_range& err) {

		cellTypeToState.resize(cellTypeToState.size() * 2);
		cellTypeToState.insert(cellTypeToState.begin() + typeID, state);
	}
}

int CellularPotts::getCellState(int typeID)
{
	return this->cellTypeToState[typeID];
}

void CellularPotts::updateBorderNearAri(int index, int old_type, int new_type)
{
	if (old_type == new_type)
		return;

	std::vector<int> Ni = this->grid.neighi(index);

	bool wasborder = this->_neighbours[index] > 0;
	this->_neighbours[index] = 0;

	for (auto ni : Ni)
	{
		int neigh_type = this->grid.pixti(ni);

		if (neigh_type != new_type)
			this->_neighbours[index]++;

		if (neigh_type == old_type) {

			if (this->_neighbours[ni]++ == 0)
				this->borderpixels.insert(ni);
		}

		if (neigh_type == new_type) {

			if (--this->_neighbours[ni] == 0) {
				this->borderpixels.remove(ni);
			}
		}
	}

	if (!wasborder && this->_neighbours[index] > 0)
		this->borderpixels.insert(index);

	if (wasborder && this->_neighbours[index] == 0)
		this->borderpixels.remove(index);

}

void CellularPotts::updateCellVolumes()
{
	// Runs every single MCS regardless of scenario, so it must not go
	// through Statistics::PixelsByCell() -- that builds a full per-cell
	// list of pixel *coordinates* (a full _pixelArray copy plus an
	// indexToPoint()+push_back per occupied pixel) just so the count of
	// entries could be taken and every coordinate immediately discarded.
	// A direct tally needs none of that: one pass over _pixelArray,
	// incrementing a per-cell-ID counter. Measured (see feature_list.txt
	// item 28) to matter a lot at low border-pixel-count/large-grid
	// scenarios (the full-grid scan dominated the whole step there) and a
	// little even in border-pixel-heavy scenarios where it's a smaller
	// share of a step's cost.
	std::vector<int> counts(this->cellTypeToKind.size(), 0);

	for (int cellId : this->grid._pixelArray)
	{
		if (cellId > 0 && cellId < (int)counts.size())
			counts[cellId]++;
	}

	for (size_t i = 1; i < this->cellTypeToKind.size(); i++)
		this->cellVolume[i] = counts[i];
}

float CellularPotts::deltaH(int sourceIndex, int targetIndex, int sourceType, int targetType)
{
	float result = 0;

	for (auto& constraint : this->contraints)
	{
		result += constraint->deltaH(sourceIndex, targetIndex, sourceType, targetType);
	}

	return result;
}

bool CellularPotts::docopy(float deltaH)
{
	if (deltaH < 0)
		return true;

	float random = (float)rand() / (float)RAND_MAX;

	return random < (float)std::exp(-deltaH / this->parameters->T);
}

//deprecated
void CellularPotts::parallelImageCalc(unsigned char* image, int startIndex, int endIndex, int i)
{
	unsigned bytePerPixel = 4;

	for (size_t x = startIndex; x < endIndex; x++)
	{
		for (size_t j = 0; j < this->grid.size.second; j++)
		{

			unsigned char* pixelOffset = image + (j + this->grid.size.second * x) * bytePerPixel;

			//TODO: GET GRID VALUE,SET COLOR ACCORDING TO
			int type = this->grid.pixti(this->grid.pointToIndex(std::pair<int, int>(x, j)));

			int r, g, b;

			if (type == 0)
			{
				r = 255;
				g = 255;
				b = 255;

			}
			else
			{
				r = 255;
				g = 40;
				b = 0;
			}


			pixelOffset[0] = r;
			pixelOffset[1] = g;
			pixelOffset[2] = b;
			pixelOffset[3] = (unsigned char)255;
		}

	}

}

void CellularPotts::setPixelI(int cellId, int sourceType)
{
	const int type_old = this->grid.pixti(cellId);

	if (type_old == sourceType)
		return;

	if (type_old > 0)
	{
		this->cellVolume[type_old]--;
		cells[type_old - 1].V--;

		// A cell whose last pixel is taken by a neighbor simply disappears --
		// its ID/kind stays recorded (cellTypeToKind, cells[]) but it owns no
		// pixels from here on, same as any other zero-volume ID. Nothing else
		// needs updating: updateCellVolumes() recomputes cellVolume from the
		// actual pixel counts every MCS anyway, rendering only ever shows
		// pixels that exist, and division checks a positive volume already.
	}

	this->grid.setpixi(cellId, sourceType);

	if (sourceType > 0)
	{
		this->cellVolume[sourceType]++;
		cells[sourceType - 1].V++;
	}

	this->updateBorderNearAri(cellId, type_old, sourceType);


	if (simTime > 0) {

		for (auto& constraint : this->contraints)
		{
			constraint->postSetpixListener(cellId, type_old, sourceType);
		}
	}

}

void CellularPotts::setPixel(std::pair<int, int> point, int sourceType)
{

	if (!settingAPixel)
	{
		settingAPixel = true;
	
		this->setPixelI(this->grid.pointToIndex(point), sourceType);
		
		settingAPixel = false;
	}
	else
	{
		while (settingAPixel)
		{
			std::this_thread::sleep_for(std::chrono::microseconds(1));
		}
	
	}
}

int CellularPotts::getCellVolume(int cellId)
{
	return this->cellVolume[cellId];
}

std::vector<int> CellularPotts::perimeterNeighbours()
{
	return _neighbours;
}

std::vector<std::pair<int, int>> CellularPotts::getBorderPixels()
{
	if (!executing)
	{
		canExecute = false;

		std::vector<std::pair<int, int>> borderPixels;

		borderPixels.resize(this->borderpixels.elements.size());

		for (size_t i = 0; i < this->borderpixels.elements.size(); i++)
		{
			const auto e = this->borderpixels.elements[i];
			const int type = this->grid.pixti(e);

			if (type != 0)
			{
				borderPixels[i] = this->grid.indexToPoint(e);
			}
			else
			{
				borderPixels[i] = std::pair<int, int>(-1, -1);
			}
		}

		canExecute = true;

		return borderPixels;

	}
	else
	{

		canExecute = false;

		while (executing)
		{
			std::this_thread::sleep_for(std::chrono::microseconds(1));
		}

		return getBorderPixels();
	}

}

bool CellularPotts::addCellAt(std::pair<int, int> point, int kind)
{
	if (point.first < 0 || point.first >= this->grid.size.first ||
	    point.second < 0 || point.second >= this->grid.size.second)
		return false;

	if (!executing)
	{
		canExecute = false;

		bool success = false;

		if (this->grid.pixti(this->grid.pointToIndex(point)) == 0)
		{
			int id = this->makeNewCellID(kind);
			this->setPixel(point, id);
			success = true;
		}

		canExecute = true;

		return success;
	}
	else
	{
		canExecute = false;

		while (executing)
		{
			std::this_thread::sleep_for(std::chrono::microseconds(1));
		}

		return addCellAt(point, kind);
	}
}

void CellularPotts::birth(int childID, int parentID)
{
	this->cells[childID - 1].birth(this->cells[parentID - 1]);
}

void CellularPotts::addPostMCstepFunction(void(*function)())
{
	postMCstepFunctions.push_back(function);
}

//deprecated
unsigned char* CellularPotts::getRenderImage()
{
	if (!executing)
	{
		canExecute = false;
		unsigned bytePerPixel = 4;
		unsigned char* image = new unsigned char[this->grid.size.first * this->grid.size.second * bytePerPixel];

		//previousImage = _strdup(image);

		std::vector<std::thread> calc_thread = std::vector<std::thread>(4);

		int init_i = 0;
		int block_size = this->grid.size.first / calc_thread.size();
		//TODO: LAST BLOCK 


		for (size_t i = 0; i < calc_thread.size(); i++)
		{

			calc_thread[i] = std::thread(&CellularPotts::parallelImageCalc, this, image, init_i, init_i + block_size, 0);

			init_i = init_i + block_size;
		}

		for (size_t i = 0; i < calc_thread.size(); i++)
		{
			if (calc_thread[i].joinable())
				calc_thread[i].join();
		}

		const auto borders = this->getBorderPixels();

		for (size_t i = 0; i < borders.size(); i++)
		{
			int r, g, b;

			r = 0;
			g = 0;
			b = 0;

			if (borders[i].first != -1 && borders[i].second != -1) {

				unsigned char* pixelOffset = (unsigned char*)image + (borders[i].second + this->grid.size.second * borders[i].first) * bytePerPixel;


				pixelOffset[0] = r;
				pixelOffset[1] = g;
				pixelOffset[2] = b;
				pixelOffset[3] = (unsigned char)255;

			}
		}

		canExecute = true;

		return image;
	}
	else
	{

		canExecute = false;

		while (executing)
		{
			std::this_thread::sleep_for(std::chrono::microseconds(1));
		}

		return getRenderImage();
	}
}

//deprecated
unsigned char* CellularPotts::getRenderImage(std::vector<int>& activityVector)
{
	if (!executing)
	{
		canExecute = false;
		unsigned bytePerPixel = 4;
		unsigned char* image = new unsigned char[this->grid.size.first * this->grid.size.second * bytePerPixel];


		std::vector<std::thread> calc_thread = std::vector<std::thread>(4);

		int init_i = 0;
		int block_size = this->grid.size.first / calc_thread.size();
		//TODO: LAST BLOCK 


		for (size_t i = 0; i < calc_thread.size(); i++)
		{

			calc_thread[i] = std::thread(&CellularPotts::parallelImageCalc, this, image, init_i, init_i + block_size, 0);

			init_i = init_i + block_size;
		}

		for (size_t i = 0; i < calc_thread.size(); i++)
		{
			if (calc_thread[i].joinable())
				calc_thread[i].join();
		}




		for (size_t i = 0; i < activityVector.size(); i++)
		{
			int value = activityVector[i];

			if (value != 0)
			{
				int r, g, b;

				r = 0;
				g = 255;
				b = 0;

				std::pair<int, int> pos = this->grid.indexToPoint(i);

				unsigned char* pixelOffset = image + (pos.second + this->grid.size.second * pos.first) * bytePerPixel;


				pixelOffset[0] = r;
				pixelOffset[1] = g;
				pixelOffset[2] = b;
				pixelOffset[3] = (unsigned char)255;
			}

		}

		const auto borders = this->getBorderPixels();

		for (size_t i = 0; i < borders.size(); i++)
		{
			int r, g, b;

			r = 0;
			g = 0;
			b = 0;

			if (borders[i].first != -1 && borders[i].second != -1) {

				unsigned char* pixelOffset = image + (borders[i].second + this->grid.size.second * borders[i].first) * bytePerPixel;


				pixelOffset[0] = r;
				pixelOffset[1] = g;
				pixelOffset[2] = b;
				pixelOffset[3] = (unsigned char)255;

			}
		}


		canExecute = true;

		return image;
	}
	else
	{

		canExecute = false;

		while (executing)
		{
			std::this_thread::sleep_for(std::chrono::microseconds(1));
		}

		return getRenderImage(activityVector);
	}
}

int CellularPotts::getCellCount()
{
	return this->cellTypeToKind.size()-1;
}

float CellularPotts::getAreaCoveredByCells()
{
	float size = 0;
	for (size_t i = 0; i < this->cellVolume.size(); i++)
	{
		size += this->cellVolume[i];
	}

	return size;
}
