#include "viewer.h"

#include <set>

class Hedge;
class Object;

class Vertex {
public:
    glm::vec3 val;
    Hedge* edge;
    Vertex* child;

    Vertex(glm::vec3 val);
    int valence();

    void Render();
    glm::vec3 Normal();
    void DrawNormal();
    std::set<Hedge*> Hedges();
};

class Face {
public:
    Hedge* edge;

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
    std::set<Face*> faces;
    std::set<Hedge*> hedges;
    std::set<Vertex*> vertices;

    Object(FILE* inputfile);
    void Render();
    void DrawNormals(int vNorms, int fNorms);
    void SetCenterSize(float *center, float *size);
    void Collapse(int nedges);
    Hedge* GetHedgeToCollapse();

    void check();
    void match_pairs();
};
