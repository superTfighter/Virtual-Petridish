#pragma once
#define CL_USE_DEPRECATED_OPENCL_2_0_APIS

#include "CL\cl.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <string> 
#include "CellularPotts.h";
#include "ActivityContraint.h"
#include <iterator>

#include <chrono>
#include <thread>



class OpenCL
{

public:
	OpenCL();
	~OpenCL();

	unsigned char* getRenderImage(CellularPotts* model, ActivityContraint* activity = nullptr);

private:

	cl::Device device;
	cl::Program program;
	cl::Context context;
	cl::CommandQueue queue;






};
