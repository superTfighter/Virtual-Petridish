#include "Display.h"
#include <iostream>

Display::Display(ID3D11Device* g_pd3dDevice, Simulation* simulation)
{
	this->g_pd3dDevice = g_pd3dDevice;
	this->simulation = simulation;


	showExampleChooser = true;
}

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

	const char* items[] = { "Simple Cell","Ising Model", "Epithelial Sheet","Cellsorting",  "Multiple Cells", "Multiple cells - small space", "Cell Division" };
	static int item_current = -1;
	ImGui::ListBox("Choose your simulation!", &item_current, items, IM_ARRAYSIZE(items), 4);

	if (item_current != -1)
	{
		showExampleChooser = false;

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
	ID3D11ShaderResourceView* my_texture = NULL;
	bool ret = LoadTexture(&my_texture, &my_image_width, &my_image_height);
	IM_ASSERT(ret);

	ImGui::SetNextWindowSize(ImVec2(width - (width * 0.3), height - (height * 0.2)));
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	bool open = true;

	ImGui::Begin("Simulation", &open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
	ImGui::Image((void*)my_texture, ImVec2((width - (width * 0.3)) - 100, (height - (height * 0.2))));
	ImGui::End();
}

void Display::showParameters()
{
	ImGui::SetNextWindowSize(ImVec2(width - (width * 0.7), height - (height * 0.2)));
	ImGui::SetNextWindowPos(ImVec2(width - (width * 0.3), 0));

	bool open = true;

	Parameters* parameters = &this->simulation->p;

	ImGui::Begin("Parameters", &open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

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

}

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

	image_data = nullptr;
	delete image_data;

	return true;
}

