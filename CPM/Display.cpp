#include "Display.h"
#include <iostream>
#include <cstdint>
#include <algorithm>
#include <map>
#include <utility>
#include <cmath>

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
	placeMode = 0;
	hunterKind = 0; // 0-based combo index -> "Kind 1"
	preyKind = 1;   // 0-based combo index -> "Kind 2"
	huntEnabled = false;
	seekResourcesEnabled = false;
	zoomLevel = 1.0f;
	panOffset = ImVec2(0.0f, 0.0f);
}
#else
Display::Display(Simulation* simulation)
{
	this->simulation = simulation;

	showExampleChooser = true;
	sandboxMode = false;
	selectedKind = 1;
	placeMode = 0;
	hunterKind = 0; // 0-based combo index -> "Kind 1"
	preyKind = 1;   // 0-based combo index -> "Kind 2"
	huntEnabled = false;
	seekResourcesEnabled = false;
	zoomLevel = 1.0f;
	panOffset = ImVec2(0.0f, 0.0f);
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

	// GetContentRegionAvail() (not a window-size-derived guess) correctly
	// excludes the title bar/padding ImGui itself reserves, so the image
	// never overflows its own window -- the old hardcoded formula caused a
	// guaranteed scrollbar on every single frame regardless of content.
	ImVec2 dispSize = ImGui::GetContentRegionAvail();

	float visibleFrac = 1.0f / zoomLevel;
	ImVec2 uv0 = panOffset;
	ImVec2 uv1 = ImVec2(panOffset.x + visibleFrac, panOffset.y + visibleFrac);

#ifdef _WIN32
	ImGui::Image((void*)my_texture, dispSize, uv0, uv1);
#else
	ImGui::Image((void*)(intptr_t)my_texture, dispSize, uv0, uv1);
#endif

	bool hovered = ImGui::IsItemHovered();
	ImVec2 itemMin = ImGui::GetItemRectMin();
	ImVec2 mousePos = ImGui::GetMousePos();
	float local_x = mousePos.x - itemMin.x;
	float local_y = mousePos.y - itemMin.y;

	// Scroll-wheel zoom, centered on the cursor (keeps the grid point under
	// the cursor fixed across the zoom change).
	if (hovered && ImGui::GetIO().MouseWheel != 0.0f && dispSize.x > 0 && dispSize.y > 0)
	{
		float cursorU = panOffset.x + (local_x / dispSize.x) * visibleFrac;
		float cursorV = panOffset.y + (local_y / dispSize.y) * visibleFrac;

		zoomLevel *= (1.0f + ImGui::GetIO().MouseWheel * 0.1f);
		zoomLevel = std::max(1.0f, std::min(zoomLevel, 8.0f));

		float newFrac = 1.0f / zoomLevel;
		panOffset.x = cursorU - (local_x / dispSize.x) * newFrac;
		panOffset.y = cursorV - (local_y / dispSize.y) * newFrac;

		panOffset.x = std::max(0.0f, std::min(panOffset.x, 1.0f - newFrac));
		panOffset.y = std::max(0.0f, std::min(panOffset.y, 1.0f - newFrac));
	}

	// Right-drag pan (left-click is already used for sandbox placement).
	if (hovered && ImGui::IsMouseDragging(ImGuiMouseButton_Right) && dispSize.x > 0 && dispSize.y > 0)
	{
		ImVec2 delta = ImGui::GetIO().MouseDelta;
		float frac = 1.0f / zoomLevel;

		panOffset.x -= (delta.x / dispSize.x) * frac;
		panOffset.y -= (delta.y / dispSize.y) * frac;

		panOffset.x = std::max(0.0f, std::min(panOffset.x, 1.0f - frac));
		panOffset.y = std::max(0.0f, std::min(panOffset.y, 1.0f - frac));
	}

	if (sandboxMode && hovered && ImGui::IsItemClicked(ImGuiMouseButton_Left)
	    && local_x >= 0 && local_x < dispSize.x && local_y >= 0 && local_y < dispSize.y)
	{
		// Map through the current zoom/pan sub-rectangle before the existing
		// transposed screen->grid conversion below.
		float u = panOffset.x + (local_x / dispSize.x) * visibleFrac;
		float v = panOffset.y + (local_y / dispSize.y) * visibleFrac;

		// The render buffer is packed x-major/y-minor, so screen-horizontal
		// maps to grid-y and screen-vertical maps to grid-x (transposed
		// from naive expectation -- see Grid::pointToIndex).
		int grid_y = (int)(u * this->simulation->model.grid.size.second);
		int grid_x = (int)(v * this->simulation->model.grid.size.first);

		grid_x = std::max(0, std::min(grid_x, this->simulation->model.grid.size.first - 1));
		grid_y = std::max(0, std::min(grid_y, this->simulation->model.grid.size.second - 1));

		if (placeMode == 0)
		{
			this->simulation->model.addCellAt(std::pair<int, int>(grid_x, grid_y), this->selectedKind);
		}
		else
		{
			// A single boosted pixel has no usable gradient beyond its
			// immediate neighbor -- ResourceSeekingConstraint's bias would
			// vanish a few pixels out, the same "gradient too localized"
			// problem ChemotaxisConstraint's scent field had. Lay down a
			// radial blob instead (linear falloff to 0 at the edge) so
			// there's a real gradient to climb from a real distance away.
			auto& grid = this->simulation->model.grid;
			const int radius = 40;
			const float peakAmount = Grid::INITIAL_RESOURCE * 2.0f;

			for (int dx = -radius; dx <= radius; dx++)
			{
				for (int dy = -radius; dy <= radius; dy++)
				{
					float dist = std::sqrt((float)(dx * dx + dy * dy));
					if (dist > radius)
						continue;

					int px = grid_x + dx;
					int py = grid_y + dy;
					if (px < 0 || px >= grid.size.first || py < 0 || py >= grid.size.second)
						continue;

					float amount = peakAmount * (1.0f - dist / radius);
					int idx = grid.pointToIndex(std::pair<int, int>(px, py));
					grid.addResourceAt(idx, amount);
				}
			}
		}
	}

	ImGui::End();
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

		ImGui::RadioButton("Place Cell", &this->placeMode, 0); ImGui::SameLine();
		ImGui::RadioButton("Place Resource", &this->placeMode, 1);
		HelpMarker("Click the Simulation image to place. Cell mode places selectedKind's cell; Resource mode boosts that pixel's nutrient level above baseline (see Kind/Hunt seeking below to make cells actually want it).");

		if (this->placeMode == 0)
		{
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
		}

		ImGui::Checkbox("Seek Resources", &this->seekResourcesEnabled);
		ImGui::SameLine(); HelpMarker("When on, every cell kind is biased to grow/move toward pixels with more nutrient. Resources start uniform, so this has no visible effect until you've placed a hotspot (Place Resource above) or cells have eaten unevenly.");
		this->simulation->p.SEEK_RESOURCES = this->seekResourcesEnabled;

		ImGui::Checkbox("Enable Hunting", &this->huntEnabled);
		ImGui::SameLine(); HelpMarker("Makes the hunter kind actively chase and consume the prey kind (real directed movement toward prey, not just winning on contact).");
		ImGui::SameLine(); ImGui::SetNextItemWidth(80); ImGui::Combo("Hunter", &this->hunterKind, "Kind 1\0Kind 2\0Kind 3\0");
		ImGui::SameLine(); ImGui::SetNextItemWidth(80); ImGui::Combo("Prey", &this->preyKind, "Kind 1\0Kind 2\0Kind 3\0");
		// Combo indices are 0-based, kinds are 1-based.
		int hunterKindValue = this->hunterKind + 1;
		int preyKindValue = this->preyKind + 1;
		if (this->huntEnabled && hunterKindValue != preyKindValue)
		{
			this->simulation->p.PREDATOR_OF = { 0,0,0,0 };
			this->simulation->p.PREDATOR_OF[hunterKindValue] = preyKindValue;
		}
		else
		{
			this->simulation->p.PREDATOR_OF = { 0,0,0,0 };
		}

		if (ImGui::Button("Clear Sandbox"))
		{
			// Resets in place (stays in sandbox view) -- unlike "Restart
			// Simulation" above, which routes back to the scenario picker.
			// setupSimulation() rebuilds p from scratch (dropping any
			// runtime PREDATOR_OF/SEEK_RESOURCES set above), so reset the
			// UI toggles too to keep them in sync with the fresh state.
			this->simulation->stopSimulation();
			this->simulation->setupSimulation(12);
			this->simulation->runSimulation();

			this->huntEnabled = false;
			this->seekResourcesEnabled = false;
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
		ImGui::Text("Cells died: %d", totalCreated - totalAlive);

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
	// GL_NEAREST (not GL_LINEAR): the source texture is native grid
	// resolution (as small as 250x250) stretched across a much larger
	// display area, so linear magnification filtering blurred the image;
	// nearest gives a crisp per-pixel look appropriate for a cell grid.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);

	*out_tex = tex;
	*out_width = image_width;
	*out_height = image_height;

	delete [] image_data;

	return true;
}
#endif

