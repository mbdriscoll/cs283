#include <cstdio>
#include <map>
#include <algorithm>
#include <vector>

#include "Object.h"

#define DEBUG 1

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

    faces = std::set<Face*>();
    hedges = std::set<Hedge*>();
    vertices = std::set<Vertex*>();

    // temporary vector to hold indexed vertices
    vector<Vertex*> vertvec;
    vertvec.reserve(numverts);

    // Scan all vertices
    for(int i = 0; i < numverts; i++) {
        float x, y, z;
        scanned = fscanf(input, "%f %f %f\n", &x, &y, &z);
        assert(scanned == 3 && "Read vertex with a non-three number of coords.");

        Vertex *newv = new Vertex(vec3(x, y, z));
        vertices.insert(newv);
        vertvec.push_back(newv);
    }

    // Scan all faces
    for(int i = 0; i < numfaces; i++) {
        int valence, vi0, vi1, vi2;
        scanned = fscanf(input, "%d %d %d %d\n", &valence, &vi0, &vi1, &vi2);
        assert(scanned == 4 && "Read a non-triangle face.");

        Face *face = new Face();
        Vertex *v0 = vertvec[vi0],
               *v1 = vertvec[vi1],
               *v2 = vertvec[vi2];

        Hedge *h0 = new Hedge(v0, NULL, face);
        Hedge *h1 = new Hedge(v1, h0,   face);
        Hedge *h2 = new Hedge(v2, h1,   face);

        v0->edge = h1;
        v1->edge = h2;
        v2->edge = h0;

        h0->next = h2;
        face->edge = h0;

        faces.insert(face);
        hedges.insert(h0);
        hedges.insert(h1);
        hedges.insert(h2);
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

        /* next pointers are circular */
        assert(h != h->next);
        assert(h != h->next->next);
        assert(h == h->next->next->next);

        /* vertex and next pointers are in different directions */
        assert(h->v == h->prev()->oppv());
        assert(h->oppv() == h->next->v);

        /* v is not oppv */
        if (h->pair != NULL)
            assert(h->v != h->oppv());

        /* vertex.edge ptr is set */
        assert(h->v->edge != NULL);

        /* edges in opp direction */
        if (h->pair != NULL) {
            assert(h->v == h->pair->oppv());
            assert(h->pair->v == h->oppv());
        }

        /* membership checks */
        assert( faces.find(h->f) != faces.end() );
        assert( hedges.find(h->next) != hedges.end() );
        assert( vertices.find(h->v) != vertices.end() );
        if (h->pair)
            assert( hedges.find(h->pair) != hedges.end() );
    }

    foreach(Vertex *v, vertices) {
        /* vertex-to-edge pointers don't align with edge-to-vertex pointers */
        assert(v->edge->next->v == v);

        /* Hedges returns hedges that point to this */
        foreach(Hedge* h, v->Hedges())
            assert(h->v == v);

        /* membership check */
        assert( hedges.find(v->edge) != hedges.end() );
    }

    foreach(Face *f, faces) {
        /* membership check */
        assert( hedges.find(f->edge) != hedges.end() );
    }

    printf("-- consistency tests passed (%d vertices, %d faces, %d hedges, %d boundary hedges) --\n",
            (int) vertices.size(),
            (int) faces.size(),
            (int) hedges.size(),
            num_boundaries);
}

Hedge::Hedge(Vertex *v, Hedge *next, Face *f) :
  v(v), next(next), f(f), pair(NULL) { }

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

inline Hedge*
Hedge::prev() {
    return this->next->next;
}

inline Vertex*
Hedge::oppv() {
    return this->next->v;
}

Vertex::Vertex(vec3 val) : edge(NULL), child(NULL), val(val)
{ }

void
Hedge::set_pair(Hedge* o) {
    this->pair = o;
    if (o != NULL)
        o->pair = this;
}

int
Vertex::valence() {
    return Hedges().size();
}

void
Object::DrawNormals(int vNorms, int fNorms) {
    glBegin(GL_LINES);
    if (vNorms) {
        glColor3f(0.0f, 0.0f, 1.0f);
        foreach(Vertex *v, vertices)
            v->DrawNormal();
    }
    if (fNorms) {
        glColor3f(0.0f, 1.0f, 0.0f);
        foreach(Face *f, faces)
            f->DrawNormal();
    }
    glEnd();

    // Draw a quad by the next edge to collapse
    Hedge* h = GetHedgeToCollapse();
    Vertex *v0 = h->v,
           *v1 = h->oppv();
    vec3 enorm = vec3(0.05f) * normalize(v0->Normal() + v1->Normal());
    vec3  ur = v0->val + enorm,
          ul = v1->val + enorm,
          lr = v0->val - enorm,
          ll = v1->val - enorm;
    glColor3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex3fv( (GLfloat*) &ur );
    glVertex3fv( (GLfloat*) &ul );
    glVertex3fv( (GLfloat*) &ll );
    glVertex3fv( (GLfloat*) &lr );
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

    vector<Hedge*> neighbors = Hedges();
    foreach(Hedge *neighbor, neighbors)
        normal += neighbor->f->Normal();

    return normalize( normal );
}

Hedge*
Object::GetHedgeToCollapse() {
    return *(hedges.begin());
}

void
Object::Collapse(int nedges) {
    for (int i = 0; i < nedges; i++) {
        Hedge *e0 = GetHedgeToCollapse();
        Hedge *e1 = e0->pair;

        Face *f0 = e0->f,
             *f1 = (e1) ? e1->f : NULL;

        Vertex *midpoint = e0->v,
               *oldpoint = e0->oppv();

        midpoint->val = vec3(0.5) * (e0->v->val + e0->oppv()->val);

        Hedge *e00 = e0,
              *e01 = e0->next,
              *e02 = e0->next->next,
              *e10 = e0->pair,
              *e11 = (e10) ? e10->next : NULL,
              *e12 = (e11) ? e11->next : NULL;

        Face *f01 = (e0->next->pair)             ? e0->next->pair->f       : NULL,
             *f02 = (e0->next->next->pair)       ? e0->next->next->pair->f : NULL,
             *f11 = (e1 && e1->next->pair)       ? e1->next->pair->f       : NULL,
             *f12 = (e1 && e1->next->next->pair) ? e1->next->next->pair->f : NULL;

        vector<Hedge*> midpointHedges = midpoint->Hedges();;

        // update vertex points from edges pointing to old vertex
        vector<Hedge*> hedgesToUpdate = oldpoint->Hedges();
        foreach(Hedge* hedge, hedgesToUpdate) {
#if DEBUG
            assert(hedge->v == oldpoint);
#endif
            hedge->v = midpoint;
        }

        // fix up pairs
        if (e01 && e01->pair) e01->pair->pair = (e02) ? e02->pair : NULL;
        if (e02 && e02->pair) e02->pair->pair = (e01) ? e01->pair : NULL;
        if (e11 && e11->pair) e11->pair->pair = (e12) ? e12->pair : NULL;
        if (e12 && e12->pair) e12->pair->pair = (e11) ? e11->pair : NULL;

        // make sure midpoint.edge is still accurate
#if DEBUG
        assert( midpoint->edge != e11 );
        assert( midpoint->edge != e12 );
        assert( midpoint->edge != e00 );
        assert( midpoint->edge != e01 );
#endif
        if ( midpoint->edge == e02 ||
             midpoint->edge == e10 ) {

            printf("warning: midpoint->edge is being deleted.\n");
            foreach(Hedge *h, midpointHedges)
                if (h != e10 && h != e02)
                    midpoint->edge = h;
        }
        assert( hedges.find(midpoint->edge) != hedges.end() );

        if (f0) faces.erase(f0);
        if (f1) faces.erase(f1);

        if (e00) hedges.erase(e00);
        if (e01) hedges.erase(e01);
        if (e02) hedges.erase(e02);
        if (e10) hedges.erase(e10);
        if (e11) hedges.erase(e11);
        if (e12) hedges.erase(e12);

        vertices.erase(oldpoint);

#ifdef DEBUG
        this->check();
#endif
    }
}

vector<Hedge*>
Vertex::Hedges() {
    Hedge *e;
    vector<Hedge*> hedges;
    hedges.push_back(edge->next);

    // forward around vertex
    for(e=edge->next->pair; e != NULL && e != edge; e=e->next->pair) {
        assert(e->next->v == this);
        hedges.push_back(e->next);
    }

    // backward if needed
    if (e == NULL) {
        for(e=edge->pair; e != NULL && e->prev() != edge; e=e->prev()->pair) {
            assert(e->v == this);
            hedges.push_back(e);
        }
    }

    return hedges;
}
