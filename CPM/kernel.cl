__constant uchar4 kindPalette[] = {
	(uchar4)(255,0,0,255),   // kind 1 - red
	(uchar4)(0,120,255,255), // kind 2 - blue
	(uchar4)(0,200,0,255),   // kind 3 - green
	(uchar4)(255,180,0,255), // kind 4 - orange
	(uchar4)(200,0,200,255), // kind 5 - magenta
};
#define KIND_PALETTE_SIZE 5

__kernel void calculate(__global char* image,__global int* pixels, __global int* kindOfCell, int sizeX, int sizeY, int y_bits)
{
	int global_id = get_global_id(0);

	int x = global_id / sizeY;
	int y = global_id % sizeY;

	int index =  (x << y_bits) + y;

	int offset = global_id * 4;

	int cellId = pixels[index];

	if(cellId == 0)
	{
		image[offset] = 255;
		image[offset + 1] = 255;
		image[offset + 2] = 255;
		image[offset + 3] = 255;
	}
	else
	{
		int kind = kindOfCell[cellId];
		int paletteIndex = ((kind - 1) % KIND_PALETTE_SIZE + KIND_PALETTE_SIZE) % KIND_PALETTE_SIZE;
		uchar4 color = kindPalette[paletteIndex];

		image[offset] = color.x;
		image[offset + 1] = color.y;
		image[offset + 2] = color.z;
		image[offset + 3] = color.w;
	}

	if(x == 0 || x == sizeX - 1 || y == 0 || y == sizeY - 1)
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