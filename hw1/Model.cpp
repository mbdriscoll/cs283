#include "viewer.h"
#include "model.h"

#include <string>
#include <cfloat>
#include <cmath>
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
    for(int i = 0; i < numfaces; i++) {
        int valence, v0, v1, v2;
        scanned = fscanf(input, "%d %d %d %d\n", &valence, &v0, &v1, &v2);
        assert(scanned == 4 && "Read a non-triangle face.");

        faces.push_back( Face(this, v0, v1, v2) );
    }
}

void
Model::Render() {
    glColor3f(1.0f, 1.0f, 0.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

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

void
Model::SetCenterSize(float *center, float *size) {
    // from osd: compute model bounding
    float min[3] = { FLT_MAX,  FLT_MAX,  FLT_MAX};
    float max[3] = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

    std::vector<Vertex>::iterator vit;
    for(vit = vertices.begin(); vit != vertices.end(); vit++) {
            min[0] = std::min(min[0], vit->x);
            max[0] = std::max(max[0], vit->x);
            min[1] = std::min(min[1], vit->y);
            max[1] = std::max(max[1], vit->y);
            min[2] = std::min(min[2], vit->z);
            max[2] = std::max(max[2], vit->z);
    }

    *size = 0.0f;
    for (int j=0; j<3; ++j) {
        center[j] = (min[j] + max[j]) * 0.5f;
        *size += (max[j]-min[j])*(max[j]-min[j]);
    }
    *size = sqrtf(*size);
}
