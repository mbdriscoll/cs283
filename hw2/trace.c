#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "image.h"

float *createMandelbrotImage(int width, int height, float xS, float yS, float rad, int maxIteration)
{
	float *buffer = (float *) malloc(width * height * sizeof(float));
	if (buffer == NULL) {
		fprintf(stderr, "Could not create image buffer\n");
		return NULL;
	}

	// Create Mandelbrot set image

	int xPos, yPos;
	float minMu = maxIteration;
	float maxMu = 0;

	for (yPos=0 ; yPos<height ; yPos++)
	{
		float yP = (yS-rad) + (2.0f*rad/height)*yPos;

		for (xPos=0 ; xPos<width ; xPos++)
		{
			float xP = (xS-rad) + (2.0f*rad/width)*xPos;

			int iteration = 0;
			float x = 0;
			float y = 0;

			while (x*x + y+y <= 4 && iteration < maxIteration)
			{
				float tmp = x*x - y*y + xP;
				y = 2*x*y + yP;
				x = tmp;
				iteration++;
			}

			if (iteration < maxIteration) {
				float modZ = sqrt(x*x + y*y);
				float mu = iteration - (log(log(modZ))) / log(2);
				if (mu > maxMu) maxMu = mu;
				if (mu < minMu) minMu = mu;
				buffer[yPos * width + xPos] = mu;
			}
			else {
				buffer[yPos * width + xPos] = 0;
			}
		}
	}

	// Scale buffer values between 0 and 1
	int count = width * height;
	while (count) {
		count --;
		buffer[count] = (buffer[count] - minMu) / (maxMu - minMu);
	}

	return buffer;
}

int main(int argc, char *argv[])
{
	// Make sure that the output filename argument has been provided
	if (argc != 2) {
		fprintf(stderr, "Please specify output file\n");
		return 1;
	}

	// Specify an output image size
	int width = 500;
	int height = 300;

	// Create a test image - in this case a Mandelbrot Set fractal
	// The output is a 1D array of floats, length: width * height
	printf("Creating Image\n");
	float *buffer = createMandelbrotImage(width, height, -0.802, -0.177, 0.011, 110);
	if (buffer == NULL) {
		return 1;
	}

	// Save the image to a PNG file
	// The 'title' string is stored as part of the PNG file
	printf("Saving PNG\n");
	int result = writeImage(argv[1], width, height, buffer, "This is my test image");

	// Free up the memorty used to store the image
	free(buffer);

	return result;
}

