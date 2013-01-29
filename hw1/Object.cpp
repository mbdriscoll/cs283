#include <map>
#include <algorithm>

#include "Object.h"

using namespace glm;
using namespace std;

typedef std::pair<Vertex*,Vertex*> VVpair;

Object::Object(FILE* input) {
    int scanned;

    // Scan OFF header
    char buf[4];
    scanned = fscanf(input, "%4s\n", (char*)  &buf);
    assert(scanned == 1 && string(buf) == "OFF" && "Could not read OFF header.");

    // Scan number of faces, verts, and numthree.
    int numverts, numfaces, numthree;
    scanned = fscanf(input, "%d %d %d\n", &numverts, &numfaces, &numthree);
    assert(scanned == 3 && "Could not read number of verts or faces from OFF file.");
    //printf("Model has %d verts, %d faces, %d numthree.\n", numverts, numfaces, numthree);

    faces = std::vector<Face*>();       faces.reserve(numfaces);
    hedges = std::vector<Hedge*>();     hedges.reserve(numfaces*3);
    vertices = std::vector<Vertex*>();  vertices.reserve(numverts);

    // Scan all vertices
    for(int i = 0; i < numverts; i++) {
        float x, y, z;
        scanned = fscanf(input, "%f %f %f\n", &x, &y, &z);
        assert(scanned == 3 && "Read vertex with a non-three number of coords.");

        vertices.push_back( new Vertex(x, y, z) );
    }

    // Scan all faces
    for(int i = 0; i < numfaces; i++) {
        int valence, vi0, vi1, vi2;
        scanned = fscanf(input, "%d %d %d %d\n", &valence, &vi0, &vi1, &vi2);
        assert(scanned == 4 && "Read a non-triangle face.");

        Face *face = new Face();
        Vertex *v0 = vertices[vi0],
               *v1 = vertices[vi1],
               *v2 = vertices[vi2];

        Hedge *h0 = new Hedge(v0, NULL, face);
        Hedge *h1 = new Hedge(v1, h0,   face);
        Hedge *h2 = new Hedge(v2, h1,   face);
        h0->next = h2;
        face->edge = h0;

        faces.push_back(face);
        hedges.push_back(h0);
        hedges.push_back(h1);
        hedges.push_back(h2);
    }

    // Match edge pairs
    map<VVpair,Hedge*> vtoe;
    foreach (Hedge* h, this->hedges)
        vtoe[VVpair(h->v,h->oppv())] = h;
    foreach (Hedge* h, this->hedges)
        h->pair = vtoe[VVpair(h->oppv(),h->v)];

    this->check();
}

void
Object::SetCenterSize(float *center, float *size) {
    // from osd: compute model bounding
    float min[3] = { FLT_MAX,  FLT_MAX,  FLT_MAX};
    float max[3] = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

    std::vector<Vertex*>::iterator vit;
    foreach(Vertex * v, vertices) {
            min[0] = std::min(min[0], v->val.x);
            max[0] = std::max(max[0], v->val.x);
            min[1] = std::min(min[1], v->val.y);
            max[1] = std::max(max[1], v->val.y);
            min[2] = std::min(min[2], v->val.z);
            max[2] = std::max(max[2], v->val.z);
    }

    *size = 0.0f;
    for (int j=0; j<3; ++j) {
        center[j] = (min[j] + max[j]) * 0.5f;
        *size += (max[j]-min[j])*(max[j]-min[j]);
    }
    *size = sqrtf(*size);
}

void
Object::Render() {
    glColor3f(0.5f, 0.2f, 0.7f);
    GLfloat materialShininess[] = {128.0f};
    GLfloat materialAmbDiff[] = {0.9f, 0.1f, 0.1f, 1.0f};
    GLfloat materialSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, materialAmbDiff);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpecular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, materialShininess);

    glBegin(GL_TRIANGLES);
    foreach(Face* f, faces)
        f->Render();
    glEnd();
}

void Object::check() {
    int num_boundaries = 0;
    int hno = 0;
    foreach(Hedge* h, this->hedges) {
        /* pair pointers are reflexive */
        if (h->pair != NULL)
            assert(h == h->pair->pair);
        else
            num_boundaries++;

        /* interior faces have pairs */
        if (h->f->interior == true)
            assert(h->pair != NULL);

        /* next pointers are circular */
        assert(h != h->next);
        assert(h != h->next->next);
        assert(h == h->next->next->next);

        /* vertex and next pointers are in the same direction */
        assert(h->v == h->next->oppv());
        assert(h->oppv() == h->prev()->v);

        /* v is not oppv */
        if (h->pair != NULL)
            assert(h->v != h->oppv());

        /* vertex.edge ptr is set */
        assert(h->v->edge != NULL);

        /* edges in opp direction */
        if (h->pair != NULL) {
            assert(h->v->val == h->pair->oppv()->val);
            assert(h->pair->v->val == h->oppv()->val);
            assert(h->v == h->pair->oppv());
            assert(h->pair->v == h->oppv());
        }
    }

    printf("-- consistency tests passed (%d vertices, %d faces, %d hedges, %d boundary hedges) --\n",
            (int) vertices.size(),
            (int) faces.size(),
            (int) hedges.size(),
            num_boundaries);
}

Hedge::Hedge(Vertex *v, Hedge *next, Face *f) :
  v(v), next(next), f(f), pair(NULL) {
      v->edge = this;
}

Face::Face() : edge(NULL)
{ }

vec3
Face::Normal() {
    vec3& v0 = this->edge->v->val;
    vec3& v1 = this->edge->next->v->val;
    vec3& v2 = this->edge->next->next->v->val;
    return normalize( cross(v2-v1, v1-v0) );
}

void
Face::Render() {
    //vec3 norm = this->Normal();
    //glNormal3fv( (GLfloat*) &norm  );

    this->edge->Render();
    this->edge->next->Render();
    this->edge->next->next->Render();
}

void
Vertex::Render() {
    vec3 norm = this->Normal();
    glNormal3fv( (GLfloat*) &norm  );
    glVertex3fv( (GLfloat*) &val );
}

void
Hedge::Render() {
    v->Render();
}

void
Object::match_pairs() {
    map<pair<Vertex*,Vertex*>,Hedge*> vtoe;
    foreach (Hedge* h, this->hedges)
        vtoe[VVpair(h->v,h->oppv())] = h;
    foreach (Hedge* h, this->hedges)
        h->pair = vtoe[VVpair(h->oppv(),h->v)];
}

inline Hedge*
Hedge::prev() {
    return this->next->next;
}

inline Vertex*
Hedge::oppv() {
    return this->prev()->v;
}

Vertex::Vertex(float x, float y, float z) : edge(NULL), child(NULL) {
    this->val = vec3(x,y,z);
}

void
Hedge::set_pair(Hedge* o) {
    this->pair = o;
    if (o != NULL)
        o->pair = this;
}

int
Vertex::valence() {
    int valence = 0;
    Hedge* current = edge;
    do {
        current = current->next->pair;
        valence++;
    } while (current != NULL && current != edge);

    /* handle boundary edges */
    if (current != edge) {
        current = edge;
        do {
            current = current->prev()->pair;
            valence++;
        } while (current != NULL && current != edge);
    }

    return valence;
}

void
Object::DrawNormals() {
    glBegin(GL_LINES);
    glColor3f(0.0f, 0.0f, 1.0f);
    foreach(Vertex *v, vertices)
        v->DrawNormal();
    glColor3f(0.0f, 1.0f, 0.0f);
    foreach(Face *f, faces)
        f->DrawNormal();
    glEnd();
}

void
Vertex::DrawNormal() {
    vec3 normal = vec3(0.5f) * normalize( Normal() );
    vec3 end = val + normal;

    glVertex3fv( (GLfloat*) &val);
    glVertex3fv( (GLfloat*) &end );
}

void
Face::DrawNormal() {
    vec3 centroid = vec3(1.0/3.0) * (edge->v->val + edge->next->v->val + edge->next->next->v->val);
    vec3 normal = vec3(0.5f) * normalize( Normal() );
    vec3 end = centroid + normal;

    glVertex3fv( (GLfloat*) &centroid );
    glVertex3fv( (GLfloat*) &end );
}

glm::vec3
Vertex::Normal() {
    vec3 normal = edge->f->Normal();;
    Hedge *e;

    // forward around vertex
    for( e=edge->next->pair; e != NULL && e != edge; e=e->next->pair)
        normal += e->f->Normal();

    // backward if needed
    if (e == NULL)
        for( e=edge->pair; e != NULL && e != edge->next; e=e->next->next->pair)
            normal += e->f->Normal();

    return normalize( normal );
}
