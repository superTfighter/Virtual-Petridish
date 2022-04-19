#pragma once
#include "SFML/Graphics.hpp"
#include "CellularPotts.h"

class SimpleDisplay
{
public:

	SimpleDisplay(CellularPotts *model, sf::RenderWindow* window)
	{
		this->_model = model;
		this->_window = window;

		renderImage.create(model->grid.size.first, model->grid.size.first, sf::Color::White);

		renderSprite.setPosition(0, 0);
		renderTexture.loadFromImage(renderImage);
		renderSprite.setTexture(renderTexture);
	}

	void render();

private:

	sf::RenderWindow *_window;
	CellularPotts *_model;

	sf::Image renderImage;
	sf::Texture renderTexture;
	sf::Sprite renderSprite;

	void setRenderImage();

	bool checkIfItsBorder(std::vector<std::pair<int, int>> borders, std::pair<int, int> point);

};

