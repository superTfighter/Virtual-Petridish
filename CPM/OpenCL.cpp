#include "OpenCL.h"


OpenCL::OpenCL()
{
	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);

	auto platform = platforms.front();

	std::vector<cl::Device> devices;

	platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);

	_ASSERT(devices.size() >= 1);

	std::cout << "Available devices: " << std::endl;

	for (auto device : devices)
	{
		auto vendor = device.getInfo<CL_DEVICE_VENDOR>();
		auto version = device.getInfo<CL_DEVICE_VERSION>();

		std::cout << "Name: " << vendor << " version: " << version << std::endl;
	}

	device = devices.front();

	std::cout << "Default device choosen: " << device.getInfo<CL_DEVICE_VENDOR>() << std::endl;

	std::ifstream invertFile("kernel.cl");

	std::string src(std::istreambuf_iterator<char>(invertFile), (std::istreambuf_iterator<char>()));

	cl::Program::Sources sources(1, std::make_pair(src.c_str(), src.length() + 1));

	context = cl::Context(device);
	program = cl::Program(context, sources);

	auto err = program.build("-cl-std=CL2.0");

	if (err)
		std::cout << "Error: " << err << std::endl;

	queue = cl::CommandQueue(context, device);
}

OpenCL::~OpenCL()
{
}

unsigned char* OpenCL::getRenderImage(CellularPotts* model, ActivityContraint* activity)
{
	//TODO:Activity parallel

	//Main Image stuff
	unsigned bytePerPixel = 4;
	unsigned char* image = new unsigned char[model->grid.size.first * model->grid.size.second * bytePerPixel]; //BUFFER ARRAY
	unsigned int* pixelsArray = (unsigned int*)model->grid._pixelArray.data(); //NOT ACTUALLY CONTAINING PIXELS VALUES

	const cl_int sizeX = model->grid.size.first;
	const cl_int sizeY = model->grid.size.second;
	const cl_int y_bits = model->grid.y_bits;
	const cl_int y_mask = model->grid.y_mask;


	cl_int err = 0;
	cl::CommandQueue queue(context, device);


	cl::Buffer memBuf(context, 0, model->grid.size.first * model->grid.size.second * bytePerPixel * sizeof(unsigned char));
	queue.enqueueWriteBuffer(memBuf, true, 0, model->grid.size.first * model->grid.size.second * bytePerPixel * sizeof(unsigned char), image);

	cl::Buffer memBuf2(context, 0, model->grid.size.first * model->grid.size.second * sizeof(unsigned int));
	queue.enqueueWriteBuffer(memBuf2, true, 0, model->grid.size.first * model->grid.size.second * sizeof(unsigned int), pixelsArray);

	cl::Kernel kernel(program, "calculate", &err);
	kernel.setArg(0, memBuf);
	kernel.setArg(1, memBuf2);
	kernel.setArg(2, sizeX);
	kernel.setArg(3, sizeY);
	kernel.setArg(4, y_bits);


	auto maxBlockNumber = device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>();

	if (model->grid.size.second < maxBlockNumber)
		maxBlockNumber = model->grid.size.second;

	queue.enqueueNDRangeKernel(kernel, cl::NullRange, model->grid.size.first * model->grid.size.second, maxBlockNumber);
	queue.enqueueReadBuffer(memBuf, true, 0, model->grid.size.first * model->grid.size.second * bytePerPixel * sizeof(unsigned char), image);


	//Border stuff
	unsigned int* bordersArray = (unsigned int*)model->borderpixels.elements.data();

	memBuf = cl::Buffer(context, 0, model->grid.size.first * model->grid.size.second * bytePerPixel * sizeof(unsigned char));
	queue.enqueueWriteBuffer(memBuf, true, 0, model->grid.size.first * model->grid.size.second * bytePerPixel * sizeof(unsigned char), image);

	memBuf2 = cl::Buffer(context, 0, model->grid.size.first * model->grid.size.second * sizeof(unsigned int));
	queue.enqueueWriteBuffer(memBuf2, true, 0, model->borderpixels.elements.size() * sizeof(unsigned int), bordersArray);

	cl::Buffer memBuf3(context, 0, model->grid.size.first * model->grid.size.second * sizeof(unsigned int));
	queue.enqueueWriteBuffer(memBuf3, true, 0, model->grid.size.first * model->grid.size.second * sizeof(unsigned int), pixelsArray);

	kernel = cl::Kernel(program, "border", &err);
	kernel.setArg(0, memBuf);
	kernel.setArg(1, memBuf2);
	kernel.setArg(2, memBuf3);
	kernel.setArg(3, y_bits);
	kernel.setArg(4, y_mask);
	kernel.setArg(5, sizeY);


	maxBlockNumber = device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>();

	if (model->borderpixels.elements.size() < maxBlockNumber)
		maxBlockNumber = model->grid.size.second;

	queue.enqueueNDRangeKernel(kernel, cl::NullRange, model->borderpixels.elements.size(), maxBlockNumber);
	queue.enqueueReadBuffer(memBuf, true, 0, model->grid.size.first * model->grid.size.second * bytePerPixel * sizeof(unsigned char), image);


	return image;
}

