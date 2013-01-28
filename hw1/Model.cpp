#include "model.h"

#include <string>
#include <cassert>
#include <cstdlib>

using namespace std;

Model::Model(FILE* input) {
    int scanned;

    // Scan OFF header
    char buf[4];
    scanned = fscanf(input, "%4s\n", (char*)  &buf);
    assert(scanned == 1 && string(buf) == "OFF" && "Could not read OFF header.");

    // Scan number of faces, verts, and numthree.
    int numverts, numfaces, numthree;
    scanned = fscanf(input, "%d %d %d\n", &numverts, &numfaces, &numthree);
    assert(scanned == 3 && "Could not read number of verts or faces from OFF file.");
    printf("Model has %d faces, %d verts, %d numthree.\n", numverts, numfaces, numthree);

    // Scan all vertices
    for(int i = 0; i < numverts; i++) {
        float x, y, z;
        scanned = fscanf(input, "%f %f %f\n", &x, &y, &z);
        assert(scanned == 3 && "Read vertex with a non-three number of coords.");
    }

    // Scan all faces
    for(int i = 0; i < numverts; i++) {
        int valence, v0, v1, v2;
        scanned = fscanf(input, "%d %d %d %d\n", &valence, &v0, &v1, &v2);
        assert(scanned == 4 && "Read a non-triangle face.");
    }
}
