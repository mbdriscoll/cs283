#include <string>

#include <cstdlib>
#include <cstdio>
#include <cmath>

#include <glm/glm.hpp>

#include "scene.h"
#include "image.h"

#define MAX_LINE_LENGTH 1024

using namespace std;
using namespace glm;

/* Global variables for problem instance */
int width = 500,
    height = 300,
    depth = 5;
char *image_fname = (char*) "image.png";

float *raytrace(int width, int height, float xS, float yS, float rad, int maxIteration)
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
				float mu = iteration - (std::log(std::log(modZ))) / std::log(2);
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
		fprintf(stderr, "Usage: %s path/to/scene.test\n", argv[0]);
        exit(1);
	}

    // parse scene file
    Scene *s = new Scene((char*) argv[1]);

    // do raytracing
	float *buffer = raytrace(width, height, -0.802, -0.177, 0.011, 110);

	// save the image
	int result = writeImage(image_fname, width, height, buffer, image_fname);

	// release image buffer
	free(buffer);

	return result;
}

