#pragma once
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <tchar.h>
#include <d3d11.h>
#include "stb_image.h"

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
	Display(ID3D11Device* g_pd3dDevice, Simulation* simulation);

	int render();
	void setSize(int width,int height);

	ID3D11Device* g_pd3dDevice;
	Simulation* simulation;
	ID3D11ShaderResourceView* my_texture;

private:
	bool showExampleChooser;
	void ExampleChooser();
	void showProject(int projectNumber);
	void showParameters();
	void showStatistics();

	int width;
	int height;

	bool LoadTexture(ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height);

};

