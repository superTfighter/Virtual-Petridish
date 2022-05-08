__kernel void calculate(__global char* image,__global int* pixels, int sizeX, int sizeY, int y_bits)
{
	int global_id = get_global_id(0);

	int x = global_id / sizeY;
	int y = global_id % sizeX;
	
	int index =  (x << y_bits) + y;

	int offset = global_id * 4;

	if((int)pixels[index] == 0)
	{
		image[offset] = 255;
		image[offset + 1] = 255;
		image[offset + 2] = 255;
		image[offset + 3] = 255;
	}
	else
	{
		image[offset] = 255;
		image[offset + 1] = 0;
		image[offset + 2] = 0;
		image[offset + 3] = 255;
	}

	if(x == 0 || x == sizeX || y == 0 || y == sizeY)
	{

		image[offset] = 0;
		image[offset + 1] = 0;
		image[offset + 2] = 0;
		image[offset + 3] = 255;

	}

		
}

__kernel void border(__global char* image,__global int* border,__global int* pixels ,  int y_bits,  int y_mask, int sizeY)
{
	int global_id = get_global_id(0);
	int index = border[global_id];

	if((int)pixels[index] != 0)
	{
		int x = (index >> y_bits);
		int y = (index & y_mask);
		
		int offset = (y + sizeY * x) * 4;
		
		image[offset] = 0;
		image[offset + 1] = 0;
		image[offset + 2] = 0;
		image[offset + 3] = 255;



	}
	
}