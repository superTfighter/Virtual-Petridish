__kernel void invert(__global int* data)
{
	int global_id = get_global_id(0);

	data[global_id] = 255;
}