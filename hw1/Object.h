#include "viewer.h"

#include <set>
#include <boost/heap/binomial_heap.hpp>


class Hedge;
class Object;
class VertexSplit;

struct QEMCompare : std::binary_function <Hedge*,Hedge*,bool> {
    bool operator() (Hedge *x, Hedge* y) const;
};

typedef boost::heap::binomial_heap<
        Hedge*,
        boost::heap::compare< QEMCompare >
    > Heap;

class Vertex {
public:
    glm::vec3 srcval; // starting location of vertex
    glm::vec3 dstval; // ending location of vertex
    int framesleft; // number of remaining animation frames
    std::set<Hedge*> edges;
    Hedge* edge();
    Vertex* child;

    Vertex(glm::vec3 val);
    int valence();

    bool Render();
    glm::vec3 Normal();
    glm::vec3 Position();
    void DrawNormal();
    void MoveTo(glm::vec3 dstval);
    void MoveTo(glm::vec4 dstval);
    void MoveFrom(glm::vec3 dstval);
    std::set<Vertex*> Vertices();
    glm::mat4 Q;
    void UpdateQ();
    void PullHedge(Hedge *newIncidentEdge, Vertex *oldpoint=NULL);
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
    Heap::handle_type handle;

    Hedge(Vertex *v, Hedge *next=NULL, Face *f=NULL);
    Hedge* prev();
    Vertex* oppv();

    void set_pair(Hedge* o);
    bool Render();
    bool IsDegenerate();
    float GetError();
    glm::vec4 GetVBar();
    glm::mat4 GetQ();
    glm::vec3 GetMidpoint();
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
    Hedge* PeekNext();

    void Pop(bool many = false);
    void Split(bool many = false);

    int check();
    void match_pairs();

    Heap queue;
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
