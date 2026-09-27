#include "Display.h"
#include <iostream>
#include <cstdint>
#include <algorithm>
#include <map>
#include <utility>

// Standard ImGui "(?)" hover-tooltip idiom, used throughout showParameters()
// so a control's purpose is visible without needing to guess from its label.
static void HelpMarker(const char* desc)
{
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(300.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

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

	const char* items[] = { "Simple Cell","Ising Model", "Epithelial Sheet","Cellsorting",  "Multiple Cells", "Wound healing", "Perimeter Demo", "Adhesion + Migration", "Cell Division", "Nutrient Foraging", "Predation", "Substates Demo", "Sandbox" };
	static int item_current = -1;
	ImGui::ListBox("Choose your simulation!", &item_current, items, IM_ARRAYSIZE(items), 4);

	if (item_current != -1)
	{
		showExampleChooser = false;
		sandboxMode = (item_current == 12);

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
			this->simulation->setupSimulation(12);
			this->simulation->runSimulation();
		}

		ImGui::Separator();
	}

	ImGui::Text("Simulation parameters");

	// A flat J/V/LAMBDA_V index encodes (kind, state) as kind*NUM_STATES+state
	// (Parameters::stateIndex) -- decode it back for readable labels. Index 0
	// is always the reserved "medium" (empty space) slot; when NUM_STATES>1,
	// indices where kind==0 but state!=0 are unused placeholder rows that no
	// real cell ever occupies (see the Substates Demo scenario comment) and
	// are skipped below to avoid showing meaningless controls.
	auto kindOf = [&](int idx) { return idx / std::max(1, parameters->NUM_STATES); };
	auto stateOf = [&](int idx) { return idx % std::max(1, parameters->NUM_STATES); };
	auto isRealSlot = [&](int idx) { return kindOf(idx) != 0 || stateOf(idx) == 0; };
	auto slotLabel = [&](int idx) -> std::string {
		int kind = kindOf(idx);
		if (kind == 0) return "medium";
		if (parameters->NUM_STATES > 1) return "kind " + std::to_string(kind) + "/state " + std::to_string(stateOf(idx));
		return "kind " + std::to_string(kind);
	};

	if (ImGui::TreeNode("Temperature"))
	{
		ImGui::SliderFloat("Simulation temp", &parameters->T, 0.0f, 1000.0f, "ratio = %.3f");
		ImGui::SameLine(); HelpMarker("How much randomness/noise drives cell movement. Higher = cells jitter and reshape more chaotically; lower = movement is more strictly governed by the adhesion/volume energy terms below (more orderly, less lifelike).");

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Adhesion"))
	{
		ImGui::TextWrapped("How costly (positive) or favorable (negative) it is for two kinds/states to touch. Cells minimize total contact cost, so a very negative value pulls two kinds together and a very positive value keeps them apart.");

		for (int i = 0; i < (int)parameters->J.size(); i++)
		{
			if (!isRealSlot(i))
				continue;

			for (int j = 0; j < (int)parameters->J[i].size(); j++)
			{
				if (!isRealSlot(j))
					continue;

				std::string label = slotLabel(i) + " <-> " + slotLabel(j);
				ImGui::SliderInt(label.c_str(), &parameters->J[i][j], -1000, 1000);
			}
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Volume"))
	{
		ImGui::TextWrapped("Each cell has a target size (in pixels) it grows/shrinks toward, and a strength controlling how strongly it resists being away from that target.");

		if (ImGui::BeginTable("split", 3))
		{
			for (int i = 0; i < (int)parameters->V.size(); i++)
			{
				if (!isRealSlot(i) || kindOf(i) == 0)
					continue;

				std::string label = slotLabel(i) + " target volume (px)";
				ImGui::TableNextColumn(); ImGui::SliderFloat(label.c_str(), &parameters->V[i], 0, 100000);
			}

			ImGui::EndTable();
		}


		if (ImGui::BeginTable("split", 3))
		{

			for (int i = 0; i < (int)parameters->LAMBDA_V.size(); i++)
			{
				if (!isRealSlot(i) || kindOf(i) == 0)
					continue;

				std::string label = slotLabel(i) + " volume constraint strength";
				ImGui::TableNextColumn(); ImGui::SliderFloat(label.c_str(), &parameters->LAMBDA_V[i], 0, 100);
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

	CellularPotts& model = this->simulation->model;
	int totalCreated = model.getCellCount();
	bool useStates = this->simulation->p.NUM_STATES > 1;

	// Tally alive counts/volumes per kind, and per (kind,state) when
	// substates are in use -- "alive" means it currently owns at least one
	// pixel (getCellVolume>0); totalCreated also counts cells that have
	// since been fully consumed/died (see CellularPotts::setPixelI).
	std::map<int, int> aliveByKind;
	std::map<int, long long> volumeByKind;
	std::map<std::pair<int, int>, int> aliveByKindState;
	std::map<std::pair<int, int>, long long> volumeByKindState;
	int totalAlive = 0;

	for (int id = 1; id <= totalCreated; id++)
	{
		int vol = model.getCellVolume(id);
		if (vol <= 0)
			continue;

		int kind = model.getCellKind(id);
		totalAlive++;
		aliveByKind[kind]++;
		volumeByKind[kind] += vol;

		if (useStates)
		{
			auto key = std::make_pair(kind, model.getCellState(id));
			aliveByKindState[key]++;
			volumeByKindState[key] += vol;
		}
	}

	static ImGuiTableFlags flags = ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_ContextMenuInBody;

	if (ImGui::BeginTable("table1", 3, flags))
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Cells alive: %d (created: %d)", totalAlive, totalCreated);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Simulation time: %d", model.simTime);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		float gridArea = (float)(model.grid.size.first * model.grid.size.second);
		float coveredPercent = gridArea > 0 ? 100.0f * model.getAreaCoveredByCells() / gridArea : 0.0f;
		ImGui::Text("Area covered: %.1f%%", coveredPercent);

		for (auto& kv : aliveByKind)
		{
			int kind = kv.first;
			int count = kv.second;
			float avgVol = count > 0 ? (float)volumeByKind[kind] / count : 0.0f;

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Kind %d: %d alive, avg volume %.0f", kind, count, avgVol);
		}

		if (useStates)
		{
			for (auto& kv : aliveByKindState)
			{
				int kind = kv.first.first;
				int state = kv.first.second;
				int count = kv.second;
				float avgVol = count > 0 ? (float)volumeByKindState[kv.first] / count : 0.0f;

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("  Kind %d / State %d: %d alive, avg volume %.0f", kind, state, count, avgVol);
			}
		}

		if (!this->simulation->p.CONSUMPTION_RATE.empty())
		{
			double totalResource = 0;
			for (float r : model.grid._resourceArray)
				totalResource += r;

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Total resource remaining: %.0f", totalResource);
		}

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

