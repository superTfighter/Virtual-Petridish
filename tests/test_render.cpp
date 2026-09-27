// Regression test for feature_list.txt item 3 (OpenCL render pipeline
// buffer-sizing/stride bugs) and item 18 (cells rendering as plain white
// medium instead of their actual color): runs a scenario with a real cell
// on it and checks Simulation::getImageData() returns a correctly-sized,
// non-null buffer that actually contains non-background (non-white) pixels,
// not just that it doesn't crash.
#include "test_framework.h"
#include "Simulation.h"

TEST_CASE("getImageData returns a correctly-sized image containing rendered cell pixels")
{
	std::vector<std::thread> threadPool(2);
	Simulation sim(&threadPool);

	sim.setupSimulation(0); // single growing cell on a 500x500 grid, see setupSimulation

	for (int step = 0; step < 20; step++)
		sim.model.monteCarloStep();

	unsigned char* image = sim.getImageData();
	CHECK_MESSAGE(image != nullptr, "getImageData() returned null");

	if (image == nullptr)
		return;

	std::pair<int, int> size = sim.getImageSize();
	CHECK(size.first > 0);
	CHECK(size.second > 0);

	const unsigned bytesPerPixel = 4;
	size_t pixelCount = (size_t)size.first * (size_t)size.second;

	bool sawNonWhitePixel = false;
	for (size_t i = 0; i < pixelCount; i++)
	{
		unsigned char r = image[i * bytesPerPixel + 0];
		unsigned char g = image[i * bytesPerPixel + 1];
		unsigned char b = image[i * bytesPerPixel + 2];

		if (r != 255 || g != 255 || b != 255)
		{
			sawNonWhitePixel = true;
			break;
		}
	}

	CHECK_MESSAGE(sawNonWhitePixel, "expected at least one non-white (rendered cell) pixel after 20 MCS steps growing from a seeded cell");

	delete[] image;
}

TEST_MAIN()
