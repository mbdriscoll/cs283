#include "viewer.h"

#include <set>

class Hedge;
class Object;
class VertexSplit;

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
    VertexSplit Collapse();
    void Split(VertexSplit& state);
    Hedge* GetHedgeToCollapse();

    int check();
    void match_pairs();
};

class VertexSplit {
  public:
    Vertex *target, *newpoint, *vA, *vB;
    Hedge *e00, *e01, *e02, *e10, *e11, *e12;
    Face *f0, *f1;
    glm::vec3 target_loc;
    VertexSplit(
            Vertex *target, Vertex *newpoint, Vertex *vA, Vertex *vB,
            Hedge *e00, Hedge *e01, Hedge *e02, Hedge *e10, Hedge *e11, Hedge *e12,
            Face *f0, Face *f1,
            glm::vec3 target_loc) :
        target(target), newpoint(newpoint), vA(vA), vB(vB),
        e00(e00), e01(e01), e02(e02), e10(e10), e11(e11), e12(e12),
        f0(f0), f1(f1), target_loc(target_loc)
    { }
};
