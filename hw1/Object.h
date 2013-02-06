#include "viewer.h"

#include <set>

class Hedge;
class Object;
class VertexSplit;

class Vertex {
public:
    glm::vec3 val; // current location of vertex
    glm::vec3 srcval; // starting location of vertex
    glm::vec3 dstval; // ending location of vertex 
    Hedge* edge;
    Vertex* child;

    Vertex(glm::vec3 val);
    int valence();

    bool Render();
    glm::vec3 Normal();
    void DrawNormal();
    void MoveTo(glm::vec3 dstval);
    std::set<Hedge*> Hedges();
};

class Face {
public:
    Hedge* edge;

    Face();
    bool Render();
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
    bool Render();
    bool IsDegenerate();
};


class Object {
public:
    std::set<Face*> faces;
    std::set<Hedge*> hedges;
    std::set<Vertex*> vertices;
    std::vector<VertexSplit*> vsplits;

    Object(FILE* inputfile);
    bool Render();
    void DrawNormals(int vNorms, int fNorms);
    void SetCenterSize(float *center, float *size);
    VertexSplit* CollapseNext();
    VertexSplit* Collapse(Hedge* e);
    Hedge* GetHedgeToCollapse();

    void Pop(bool many = false);
    void Split(bool many = false);

    int check();
    void match_pairs();
};

class VertexSplit {
  public:
    /* Lots of this can be derived but it's complicated when the mesh has been mutated,
     * so let's just store it to simplify things. */
    Vertex *target, *newpoint, *vA, *vB;
    Hedge *e00, *e01, *e02, *e10, *e11, *e12;
    Face *f0, *f1;
    glm::vec3 target_loc;
    std::set<Hedge*> targetHedges, newpointHedges;

    VertexSplit(Hedge *e00);
    void Apply(Object* o);

    VertexSplit *degenA;
    VertexSplit *degenB;
};
