#include "viewer.h"

#include <vector>

class Hedge;
class Object;

class Vertex {
public:
    glm::vec3 val;
    Hedge* edge;
    Vertex* child;

    Vertex(float x, float y, float z);
    int valence();

    void Render();
    glm::vec3 Normal();
    void DrawNormal();
};

class Face {
public:
    Hedge* edge;
    bool interior;

    Face();
    void Render();
    void DrawNormal();
    glm::vec3 Normal();
};

class Hedge {
public:
    Face* f;
    Hedge* next;
    Hedge* pair;
    Vertex* v;

    Hedge(Vertex *v, Hedge *next, Face *f);
    Hedge* prev();
    Vertex* oppv();

    void set_pair(Hedge* o);
    void Render();
};


class Object {
public:
    std::vector<Face*> faces;
    std::vector<Hedge*> hedges;
    std::vector<Vertex*> vertices;

    Object(FILE* inputfile);
    void Render();
    void DrawNormals(int vNorms, int fNorms);
    void SetCenterSize(float *center, float *size);

    void check();
    void match_pairs();
};
