#pragma once
#include "imgui.h"
#ifdef _WIN32
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include <tchar.h>
#include <d3d11.h>
#else
#include <GL/gl.h>
#endif

#include "Grid.h"
#include "GridManadger.h"
#include "CellularPotts.h"
#include "AdhesionConstraint.h"
#include "VolumeConstraint.h"
#include "PerimeterConstraint.h"
#include "PixelsByCell.h"
#include "Centroids.h"
#include "Simulation.h"

class Display
{
public:
#ifdef _WIN32
	Display(ID3D11Device* g_pd3dDevice, Simulation* simulation);
#else
	Display(Simulation* simulation);
#endif

	int render();
	void setSize(int width,int height);

#ifdef _WIN32
	ID3D11Device* g_pd3dDevice;
#endif
	Simulation* simulation;
#ifdef _WIN32
	ID3D11ShaderResourceView* my_texture;
#else
	unsigned int my_texture; // GLuint texture id (0 = none)
#endif

private:
	bool showExampleChooser;
	void ExampleChooser();
	void showProject(int projectNumber);
	void showParameters();
	void showStatistics();

	int width;
	int height;

#ifdef _WIN32
	bool LoadTexture(ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height);
#else
	bool LoadTexture(unsigned int* out_tex, int* out_width, int* out_height);
#endif

};

