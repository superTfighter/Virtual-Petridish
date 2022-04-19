#include "SimpleDisplay.h"

void SimpleDisplay::render()
{
	this->setRenderImage();

	sf::Event event;

	while (this->_window->pollEvent(event))
	{
		// Request for closing the window
		if (event.type == sf::Event::Closed)
			this->_window->close();
	}

	this->_window->clear(sf::Color::White);

	//DRAW HERE!!
	_window->draw(renderSprite);
	//

	this->_window->setActive();
	this->_window->display();
}

//TODO: CHANGE COLOR ACCORDING TO CELLTYPE
void SimpleDisplay::setRenderImage()
{
	int x, y, r, g, b;

	const auto borders = this->_model->getBorderPixels();

	for (size_t i = 0; i < this->_model->grid.size.first; i++)
	{
		for (size_t j = 0; j < this->_model->grid.size.second; j++)
		{
			int type = this->_model->grid.pixti(this->_model->grid.pointToIndex(std::pair<int, int>(i, j)));

			sf::Color color;

			if (type == 0)
			{
				r = 255;
				g = 255;
				b = 255;

				color = sf::Color(r, g, b);
			}
			else
			{
				r = 0;
				g = (255 - 128) * (type - 0) / (this->_model->cells.size() - 0) + 128;
				b = 0;
			
				color = sf::Color(r, g, b);
			}

			x = i;
			y = (this->_model->grid.size.second - 1) - j;

			renderImage.setPixel(i, j, color);
		}
	}

	//RENDER BORDERS SEPARERTLY
	for (size_t i = 0; i < borders.size(); i++)
	{
		sf::Color color;

		r = 0;
		g = 0;
		b = 0;

		color = sf::Color(r, g, b);

		if(borders[i].first != -1 && borders[i].second != -1)
			renderImage.setPixel(borders[i].first, borders[i].second, color);
	}

	renderTexture.loadFromImage(renderImage);
	renderSprite.setTexture(renderTexture);

	renderSprite.setScale(static_cast<float>(_window->getView().getSize().x) / renderSprite.getTexture()->getSize().x, static_cast<float>(_window->getView().getSize().y) / renderSprite.getTexture()->getSize().y);

}

bool SimpleDisplay::checkIfItsBorder(std::vector<std::pair<int, int>> borders, std::pair<int, int> point)
{
	for (size_t i = 0; i < borders.size(); i++)
	{
		if (borders[i].first != borders[i].first == point.first && borders[i].second == point.second)
			return true;
	}

	return false;
}

