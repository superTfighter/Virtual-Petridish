#include "Display.h"
#include <iostream>
#include <cstdint>
#include <algorithm>

#ifdef _WIN32
Display::Display(ID3D11Device* g_pd3dDevice, Simulation* simulation)
{
	this->g_pd3dDevice = g_pd3dDevice;
	this->simulation = simulation;


	showExampleChooser = true;
	sandboxMode = false;
	selectedKind = 1;
}
#else
Display::Display(Simulation* simulation)
{
	this->simulation = simulation;

	showExampleChooser = true;
	sandboxMode = false;
	selectedKind = 1;
}
#endif

int Display::render()
{
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);


	// Start the Dear ImGui frame
	ImGui::NewFrame();

	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).

	//ImGui::ShowDemoWindow();


	if (!showExampleChooser) {
		showProject(1);
		showParameters();
		showStatistics();

	}


	if (showExampleChooser)
		ExampleChooser();


	// Rendering
	ImGui::Render();

	return 0;
}

void Display::setSize(int width, int height)
{
	this->width = width;
	this->height = height;
}

void Display::ExampleChooser()
{
	ImGui::SetNextWindowSize(ImVec2(width, height - (height * 0.2)));
	ImGui::SetNextWindowPos(ImVec2(0, 0));

	bool open = true;

	ImGui::Begin("Project Chooser", &open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

	ImGui::Text("Please choose one!");

	ImGui::Separator();

	const char* items[] = { "Simple Cell","Ising Model", "Epithelial Sheet","Cellsorting",  "Multiple Cells", "Wound healing", "Perimeter Demo", "Adhesion + Migration", "Cell Division", "Nutrient Foraging", "Predation", "Sandbox" };
	static int item_current = -1;
	ImGui::ListBox("Choose your simulation!", &item_current, items, IM_ARRAYSIZE(items), 4);

	if (item_current != -1)
	{
		showExampleChooser = false;
		sandboxMode = (item_current == 11);

		simulation->setupSimulation(item_current);

		simulation->runSimulation();

		item_current = -1;
	}

	ImGui::End();
}

void Display::showProject(int projectNumber)
{
	int my_image_width = 0;
	int my_image_height = 0;
#ifdef _WIN32
	my_texture = NULL;
#else
	my_texture = 0;
#endif
	bool ret = LoadTexture(&my_texture, &my_image_width, &my_image_height);
	IM_ASSERT(ret);

	ImGui::SetNextWindowSize(ImVec2(width - (width * 0.3), height - (height * 0.2)));
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	bool open = true;

	ImGui::Begin("Simulation", &open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
#ifdef _WIN32
	ImGui::Image((void*)my_texture, ImVec2((width - (width * 0.3)) - 100, (height - (height * 0.2)))); //TODO: REVISE
#else
	ImGui::Image((void*)(intptr_t)my_texture, ImVec2((width - (width * 0.3)) - 100, (height - (height * 0.2)))); //TODO: REVISE
#endif

	if (sandboxMode && ImGui::IsItemHovered() && ImGui::IsItemClicked(ImGuiMouseButton_Left))
	{
		ImVec2 itemMin = ImGui::GetItemRectMin();
		ImVec2 mousePos = ImGui::GetMousePos();

		float local_x = mousePos.x - itemMin.x;
		float local_y = mousePos.y - itemMin.y;

		float W_disp = (width - (width * 0.3f)) - 100;
		float H_disp = (height - (height * 0.2f));

		if (local_x >= 0 && local_x < W_disp && local_y >= 0 && local_y < H_disp)
		{
			float u = local_x / W_disp;
			float v = local_y / H_disp;

			// The render buffer is packed x-major/y-minor, so screen-horizontal
			// maps to grid-y and screen-vertical maps to grid-x (transposed
			// from naive expectation -- see Grid::pointToIndex).
			int grid_y = (int)(u * this->simulation->model.grid.size.second);
			int grid_x = (int)(v * this->simulation->model.grid.size.first);

			grid_x = std::max(0, std::min(grid_x, this->simulation->model.grid.size.first - 1));
			grid_y = std::max(0, std::min(grid_y, this->simulation->model.grid.size.second - 1));

			this->simulation->model.addCellAt(std::pair<int, int>(grid_x, grid_y), this->selectedKind);
		}
	}

	//ImGui::Image((void*)my_texture, ImVec2(my_image_width, my_image_height));
	ImGui::End();


	 //my_texture = NULL;
	 //delete my_texture;
}

void Display::showParameters()
{
	ImGui::SetNextWindowSize(ImVec2(width - (width * 0.7), height - (height * 0.2)));
	ImGui::SetNextWindowPos(ImVec2(width - (width * 0.3), 0));

	bool open = true;

	Parameters* parameters = &this->simulation->p;

	ImGui::Begin("Parameters", &open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

	if (ImGui::Button("Restart Simulation"))
	{
		// Stops (and joins) the running Monte Carlo thread, then routes back
		// to the scenario picker -- picking a scenario there already does a
		// full setupSimulation()+runSimulation() reset.
		this->simulation->stopSimulation();
		this->showExampleChooser = true;
	}

	ImGui::Separator();

	// Resets to 0 (fastest) on every Restart/scenario-switch/Clear Sandbox,
	// since those all reconstruct CellularPotts from scratch -- intentional,
	// not a bug.
	int speed = this->simulation->model.stepDelayMs.load();
	if (ImGui::SliderInt("Simulation Speed (delay ms, 0=fastest)", &speed, 0, 200))
	{
		this->simulation->model.stepDelayMs.store(speed);
	}

	ImGui::Separator();

	if (sandboxMode)
	{
		ImGui::Text("Sandbox Controls");

		ImGui::RadioButton("Kind 1", &this->selectedKind, 1); ImGui::SameLine();
		ImGui::RadioButton("Kind 2", &this->selectedKind, 2); ImGui::SameLine();
		ImGui::RadioButton("Kind 3", &this->selectedKind, 3);

		if (ImGui::Button("Add Random Cell"))
		{
			int max_attempts = 1000;
			auto& grid = this->simulation->model.grid;

			for (int attempt = 0; attempt < max_attempts; attempt++)
			{
				std::pair<int, int> point(rand() % grid.size.first, rand() % grid.size.second);

				if (grid.pixti(grid.pointToIndex(point)) == 0)
				{
					this->simulation->model.addCellAt(point, this->selectedKind);
					break;
				}
			}
		}

		if (ImGui::Button("Clear Sandbox"))
		{
			// Resets in place (stays in sandbox view) -- unlike "Restart
			// Simulation" above, which routes back to the scenario picker.
			this->simulation->stopSimulation();
			this->simulation->setupSimulation(11);
			this->simulation->runSimulation();
		}

		ImGui::Separator();
	}

	ImGui::Text("Simulation parameters");


	if (ImGui::TreeNode("Temperature"))
	{
		ImGui::SliderFloat("Simulation temp", &parameters->T, 0.0f, 1000.0f, "ratio = %.3f");

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Adhesion"))
	{
		for (int i = 0; i < parameters->J.size(); i++)
		{
			for (int j = 0; j < parameters->J[i].size(); j++)
			{

				ImGui::SliderInt((std::to_string(i) + " -> " + std::to_string(j)).c_str(), &parameters->J[i][j], -1000, 1000);
			}
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Volume"))
	{

		if (ImGui::BeginTable("split", 3))
		{
			for (int i = 0; i < parameters->V.size(); i++)
			{
				ImGui::TableNextColumn(); ImGui::SliderFloat((std::to_string(i) + ". celltype max volume").c_str(), &parameters->V[i], 0, 100000);
			}

			ImGui::EndTable();
		}


		if (ImGui::BeginTable("split", 3))
		{

			for (int i = 0; i < parameters->LAMBDA_V.size(); i++)
			{
				ImGui::TableNextColumn(); ImGui::SliderFloat((std::to_string(i) + ". celltype volume change").c_str(), &parameters->LAMBDA_V[i], 0, 100);
			}

			ImGui::EndTable();
		}

		ImGui::TreePop();
	}

	ImGui::End();

	parameters = nullptr;
	delete[] parameters;

}

void Display::showStatistics()
{
	ImGui::SetNextWindowSize(ImVec2(width, height - (height * 0.8)));
	ImGui::SetNextWindowPos(ImVec2(0, height - (height * 0.2)));

	bool open = true;
	ImGui::Begin("Statistics", &open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

	static ImGuiTableFlags flags = ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_ContextMenuInBody;

	if (ImGui::BeginTable("table1", 3, flags))
	{


		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Number of cells: %d", this->simulation->model.getCellCount());

		ImGui::TableSetColumnIndex(1);

		ImGui::TableSetColumnIndex(2);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Simulation time: %d", this->simulation->model.simTime);

		ImGui::TableSetColumnIndex(1);

		ImGui::TableSetColumnIndex(2);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("AreaCovered by cells: %d / %f : %f percent", this->simulation->model.grid.size.first * this->simulation->model.grid.size.second, this->simulation->model.getAreaCoveredByCells(), this->simulation->model.getAreaCoveredByCells() / (this->simulation->model.grid.size.first * this->simulation->model.grid.size.second));

		ImGui::TableSetColumnIndex(1);

		ImGui::TableSetColumnIndex(2);



		/*for (int row = 0; row < 5; row++)
		{
			
			for (int column = 0; column < 3; column++)
			{
				ImGui::TableSetColumnIndex(column);
				ImGui::Text("Hello %d,%d", column, row);
			}
		}*/
		ImGui::EndTable();
	}
	
	ImGui::End();


}

#ifdef _WIN32
bool Display::LoadTexture(ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height)
{
	// Load from disk into a raw RGBA buffer
	int image_width = this->simulation->getImageSize().first;
	int image_height = this->simulation->getImageSize().second;
	int channels = 0;
	unsigned char* image_data = this->simulation->getImageData();
	if (image_data == NULL)
		return false;

	// Create texture
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = image_width;
	desc.Height = image_height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;

	ID3D11Texture2D* pTexture = NULL;
	D3D11_SUBRESOURCE_DATA subResource;
	subResource.pSysMem = image_data;
	subResource.SysMemPitch = desc.Width * 4;
	subResource.SysMemSlicePitch = 0;
	g_pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);

	// Create texture view
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = desc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	g_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, out_srv);
	pTexture->Release();

	*out_width = image_width;
	*out_height = image_height;

	delete [] image_data;


	return true;
}
#else
bool Display::LoadTexture(unsigned int* out_tex, int* out_width, int* out_height)
{
	int image_width = this->simulation->getImageSize().first;
	int image_height = this->simulation->getImageSize().second;
	unsigned char* image_data = this->simulation->getImageData();
	if (image_data == NULL)
		return false;

	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);

	*out_tex = tex;
	*out_width = image_width;
	*out_height = image_height;

	delete [] image_data;

	return true;
}
#endif

