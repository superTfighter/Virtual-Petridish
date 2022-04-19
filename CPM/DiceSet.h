#pragma once
#include <vector>
#include <map>
#include <random>
#include <iostream>

class DiceSet
{
public:
	DiceSet();

	std::vector<int> elements;

	//UNIQUE ID _ LENGHTS
	std::map<int, int> indices;

	float length;

	bool contains(int uniqueID);
	void insert(int uniqueID);
	void remove(int uniqueID);
	int sample();

private:

	void insertCertainValue(int uniqueID, int lenght);
	
	std::mt19937 generator;
};

