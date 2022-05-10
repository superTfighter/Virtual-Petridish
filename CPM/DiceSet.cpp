#include "DiceSet.h"


DiceSet::DiceSet()
{
	this->indices = std::map<int, int>();
	this->elements = std::vector<int>();

	this->length = 0;

	this->generator = std::mt19937(time(NULL));
}

bool DiceSet::contains(int uniqueID)
{
	if(indices.count(uniqueID))
	{
		return true;
	}

	return false;

}

void DiceSet::insert(int uniqueID)
{
	if (this->contains(uniqueID))
		return;

	this->indices.insert(std::pair<int, int>(uniqueID,length));
	this->elements.push_back(uniqueID);
	this->length++;

}

void DiceSet::remove(int uniqueID)
{
	if (!this->contains(uniqueID))
		return;

	int i = this->indices[uniqueID]; //THIS IS A LENGHT STORED 
	int e = this->elements.back(); //THIS IS AN ID
	
	this->indices.erase(uniqueID);
	this->elements.pop_back();
	this->length--;

	if (e == uniqueID)
		return;	

	this->insertCertainValue(e, i);
}

int DiceSet::sample()
{
	const int size = this->elements.size();

	std::uniform_real_distribution<double> dis(0, size);

	if(size <= 0)
	{	
		throw "Error";
	}

	int index = dis(this->generator);

	return elements[index];
}

void DiceSet::insertCertainValue(int uniqueID, int lenght)
{
	if (this->contains(uniqueID))
	{
		this->indices[uniqueID] = lenght;
	}
	else
	{
		this->indices.insert(std::pair<int, int>(uniqueID, lenght));
	}

	if (lenght < elements.size())
	{
		elements[lenght] = uniqueID;
	}
	else
	{
		elements.resize((int)(lenght + 1));
		elements[lenght] = uniqueID;
	}
	
	return;
}

