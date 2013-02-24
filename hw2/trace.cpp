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
	if (argc < 2) {
		fprintf(stderr, "Usage: %s path/to/scene.test\n", argv[0]);
        exit(1);
	}

    bool preview = false;

    int i = 1;
    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i++],"-p"))
            preview = true;
    }

    // parse scene file
    Scene *s = new Scene((char*) argv[1]);

    if (preview)
        s->Preview();
    else
        s->RayTrace();

    // release resources
    delete s;

	return 0;
}

