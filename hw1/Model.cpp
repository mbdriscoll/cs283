#include "viewer.h"
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

    vertices = std::vector<Vertex>();  vertices.reserve(numverts);
    faces = std::vector<Face>();       faces.reserve(numfaces);

    // Scan all vertices
    for(int i = 0; i < numverts; i++) {
        float x, y, z;
        scanned = fscanf(input, "%f %f %f\n", &x, &y, &z);
        assert(scanned == 3 && "Read vertex with a non-three number of coords.");

        vertices.push_back( Vertex(this, x, y, z) );
    }

    // Scan all faces
    for(int i = 0; i < numverts; i++) {
        int valence, v0, v1, v2;
        scanned = fscanf(input, "%d %d %d %d\n", &valence, &v0, &v1, &v2);
        assert(scanned == 4 && "Read a non-triangle face.");

        faces.push_back( Face(this, v0, v1, v2) );
    }
}

void
Model::Render() {
    glColor3f(1.0f, 1.0f, 0.5f);

    std::vector<Face>::iterator fit;
    for(fit = faces.begin(); fit != faces.end(); fit++)
        fit->Render();
}

void
Model::Vertex::Render() {
    glVertex3f(x, y, z);
}

void
Model::Face::Render() {
    glBegin(GL_TRIANGLES);
    model->vertices[v0].Render();
    model->vertices[v1].Render();
    model->vertices[v2].Render();
    glEnd();
}
