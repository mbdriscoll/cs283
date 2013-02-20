#include <cstdio>
#include <map>
#include <algorithm>
#include <vector>
#include <cstdio>

#include "Object.h"

#define DEBUG 0

#if DEBUG
    #define DEBUG_ASSERT(cnd) assert((cnd));
#else
    #define DEBUG_ASSERT(cmd) {};
#endif

using namespace glm;
using namespace std;

extern int g_qem;

typedef std::pair<Vertex*,Vertex*> VVpair;

void glm_print(glm::vec3 v) {
    printf("{%1.3f,%1.3f,%1.3f}\n", v.x, v.y, v.z);
}

void glm_print(glm::vec4 v) {
    printf("{%1.3f,%1.3f,%1.3f,%1.3f}\n", v.x, v.y, v.z, v.w);
}
void glm_print(glm::mat4 m) {
    printf("{{%1.3f,%1.3f,%1.3f,%1.3f},{%1.3f,%1.3f,%1.3f,%1.3f},{%1.3f,%1.3f,%1.3f,%1.3f},{%1.3f,%1.3f,%1.3f,%1.3f}}\n",
            m[0][0], m[1][0], m[2][0], m[3][0],
            m[0][1], m[1][1], m[2][1], m[3][1],
            m[0][2], m[1][2], m[2][2], m[3][2],
            m[0][3], m[1][3], m[2][3], m[3][3]);

}

glm::vec3 dehomogenize(glm::vec4 v) {
    return vec3(1.0f/v.w) * vec3(v.x, v.y, v.z);
}

glm::vec4 homogenize(glm::vec4 v) {
    return vec4(v.x/v.w, v.y/v.w, v.z/v.w, 1.0f);
}

glm::vec4 homogenize(glm::vec3 v) {
    return vec4(v.x, v.y, v.z, 1.0f);
}


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

    faces = std::set<Face*>();
    hedges = std::set<Hedge*>();
    vertices = std::set<Vertex*>();

    // temporary vector to hold indexed vertices
    vector<Vertex*> vertvec;
    vertvec.reserve(numverts);
    vsplits.reserve(numverts);

    // Scan all vertices
    for(int i = 0; i < numverts; i++) {
        double x, y, z;
        scanned = fscanf(input, "%lf %lf %lf\n", &x, &y, &z);
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
        Hedge *h2 = new Hedge(v2, h0,   face);
        Hedge *h1 = new Hedge(v1, h2,   face);
        h0->next = h1;

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

    DEBUG_ASSERT( this->check() );

    // Compute Q values
    foreach(Vertex* v, vertices)
        v->UpdateQ();

    // Put edges in priority queue
    foreach(Hedge* e, hedges)
        e->handle = queue.push(e);
}


void
Object::SetCenterSize(float *center, float *size) {
    // from osd: compute model bounding
    float min[3] = { FLT_MAX,  FLT_MAX,  FLT_MAX};
    float max[3] = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

    foreach(Vertex * v, vertices) {
            min[0] = std::min(min[0], v->dstval.x);
            max[0] = std::max(max[0], v->dstval.x);
            min[1] = std::min(min[1], v->dstval.y);
            max[1] = std::max(max[1], v->dstval.y);
            min[2] = std::min(min[2], v->dstval.z);
            max[2] = std::max(max[2], v->dstval.z);
    }

    *size = 0.0f;
    for (int j=0; j<3; ++j) {
        center[j] = (min[j] + max[j]) * 0.5f;
        *size += (max[j]-min[j])*(max[j]-min[j]);
    }
    *size = sqrtf(*size);
}

bool
Object::Render() {
    bool done = true;

    glBegin(GL_TRIANGLES);
    foreach(Face* f, faces)
        done &= f->Render();
    glEnd();

    return done;
}

int Object::check() {

    int num_boundaries = 0;
    int hno = 0;
    foreach(Hedge* h, this->hedges) {
        /* next is defined */
        assert(h->next);
        assert(h->next->next);
        assert(h->next->next->next == h);

        /* pair pointers are reflexive */
        if (h->pair != NULL)
            assert(h == h->pair->pair);
        else
            num_boundaries++;

        /* self isn't a pair */
        assert(h->pair != h);

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

        /* edges in opp direction */
        if (h->pair != NULL) {
            assert(h->v == h->pair->oppv());
            assert(h->pair->v == h->oppv());
        }

        /* neighbors around vertex have same vertex */
        if (h->pair != NULL) {
            assert(h->next->v == h->pair->v);
            assert(h->v == h->pair->next->v);
        }

        /* membership checks */
        assert( faces.find(h->f) != faces.end() );
        assert( hedges.find(h->next) != hedges.end() );
        if ( vertices.find(h->v) == vertices.end() ) {
            printf("missing vertex %lx\n", (long int) h->v);
            assert( vertices.find(h->v) != vertices.end() );
        }
        if (h->pair)
            assert( hedges.find(h->pair) != hedges.end() );
    }

    foreach(Vertex *v, vertices) {
        /* Hedges returns hedges that point to this */
        foreach(Hedge* h, v->edges)
            assert(h->v == v);

        /* vertices have edges */
        assert(v->edges.size() > 0);

        /* membership check */
        foreach(Hedge* e, v->edges)
            assert( hedges.find(e) != hedges.end() );
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

    return 1;
}

Hedge::Hedge(Vertex *v, Hedge *next, Face *f) :
    v(v), next(next), f(f), pair(NULL)
{
    v->PullHedge(this);
    f->edge = this;
}

Face::Face() : edge(NULL)
{ }

vec3
Face::Normal() {
    vec3 v0 = this->edge->v->dstval;
    vec3 v1 = this->edge->next->v->dstval;
    vec3 v2 = this->edge->next->next->v->dstval;
    return normalize( cross(v1-v0, v2-v1) );
}

vec3
Face::CurrentNormal() {
    vec3 v0 = this->edge->v->Position();
    vec3 v1 = this->edge->next->v->Position();
    vec3 v2 = this->edge->next->next->v->Position();
    return normalize( cross(v1-v0, v2-v1) );
}

bool
Face::Render() {
    bool done = true;
    done &= this->edge->Render();
    done &= this->edge->next->Render();
    done &= this->edge->next->next->Render();
    return done;
}

bool
Vertex::Render() {
    vec3 pos = this->Position(),
         norm = this->CurrentNormal();
    glNormal3fv( (GLfloat*) &norm  );
    glVertex3fv( (GLfloat*) &pos );

    if (framesleft > 0)
        framesleft -= 1;

    return framesleft == 0;
}

bool
Hedge::Render() {
    return v->Render();
}

inline Hedge*
Hedge::prev() {
    return this->next->next;
}

inline Vertex*
Hedge::oppv() {
    return this->next->v;
}

Vertex::Vertex(vec3 val) :
    dstval(val), srcval(val), framesleft(0)
{ }

void
Hedge::SetPair(Hedge* o) {
    this->pair = o;
    if (o != NULL)
        o->pair = this;
}

int
Vertex::valence() {
    return edges.size();
}

void
Object::DrawPoints() {
    glPointSize(5.0f);
    glColor3f(1.0f, 1.0f, 1.0f); // yellow

    glBegin(GL_POINTS);
    foreach(Vertex* v, vertices)
        glVertex3fv(&v->dstval.x);
    glEnd();
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
}

void
Vertex::DrawNormal() {
    vec3 norm = vec3(0.5f) * CurrentNormal();
    vec3 pos = Position();
    vec3 end = pos + norm;

    glVertex3fv( (GLfloat*) &pos );
    glVertex3fv( (GLfloat*) &end );
}

void
Face::DrawNormal() {
    vec3 centroid = vec3(1.0/3.0) *
        (edge->v->Position()+ edge->next->v->Position()+ edge->next->next->v->Position());
    vec3 normal = vec3(0.5f) * CurrentNormal();
    vec3 end = centroid + normal;

    glVertex3fv( (GLfloat*) &centroid );
    glVertex3fv( (GLfloat*) &end );
}

glm::vec3
Vertex::Normal() {
    vec3 normal(0.0f);
    Hedge *e;

    assert(edges.size() > 0);
    foreach(Hedge *neighbor, edges)
        normal += neighbor->f->Normal();

    return normalize( normal );
}

glm::vec3
Vertex::CurrentNormal() {
    vec3 normal(0.0f);
    Hedge *e;

    assert(edges.size() > 0);
    foreach(Hedge *neighbor, edges)
        normal += neighbor->f->CurrentNormal();

    return normalize( normal );
}

Hedge*
Object::PeekNext() {
    if (g_qem)
        return queue.top();
    else
        return *(hedges.begin());
}

VertexSplit*
Object::CollapseNext() {
    Hedge *e0 = PeekNext();
    return this->Collapse(e0, e0->GetVBar());
}

VertexSplit*
Object::Collapse(Hedge *e00, vec4 newloc) {
    // -------------------------------------------------------
    // save state

    Hedge *e01 = e00->next,
          *e02 = e01->next,
          *e10 = e00->pair,
          *e11 = (e10) ? e10->next : NULL,
          *e12 = (e11) ? e11->next : NULL;

    Face *f0 = (e00) ? e00->f : NULL,
         *f1 = (e10) ? e10->f : NULL;

    Vertex *midpoint = e00->v,
           *oldpoint = e00->oppv(),
           *vA = (e02) ? e02->v : NULL,
           *vB = (e12) ? e12->v : NULL;

    bool delete_mp = false,
         delete_va = false,
         delete_vb = false;

    VertexSplit *state = new VertexSplit(e00);

    if (e00) DEBUG_ASSERT(midpoint->edges.find(e00) != midpoint->edges.end());
    if (e11) DEBUG_ASSERT(midpoint->edges.find(e11) != midpoint->edges.end());
    if (e01) DEBUG_ASSERT(oldpoint->edges.find(e01) != oldpoint->edges.end());
    if (e10) DEBUG_ASSERT(oldpoint->edges.find(e10) != oldpoint->edges.end());
    if (e02) DEBUG_ASSERT(vA->edges.find(e02) != vA->edges.end());
    if (e12) DEBUG_ASSERT(vB->edges.find(e12) != vB->edges.end());

    // -------------------------------------------------------
    // make updates

    midpoint->MoveTo( newloc );

    // fix up pairs
    if (e01 && e01->pair) e01->pair->SetPair(e02->pair);
    if (e02 && e02->pair) e02->pair->SetPair(e01->pair);
    if (e11 && e11->pair) e11->pair->SetPair(e12->pair);
    if (e12 && e12->pair) e12->pair->SetPair(e11->pair);

    // update vertex points from edges pointing to old vertex
    if (e00) midpoint->edges.erase(e00);
    if (e11) midpoint->edges.erase(e11);
    if (e01) oldpoint->edges.erase(e01);
    if (e10) oldpoint->edges.erase(e10);
    if (vA)  vA->edges.erase(e02);
    if (vB)  vB->edges.erase(e12);

    foreach(Hedge* hedge, set<Hedge*>(oldpoint->edges))
        midpoint->PullHedge(hedge, oldpoint);

    if (e00) DEBUG_ASSERT(midpoint->edges.find(e00) == midpoint->edges.end());
    if (e01) DEBUG_ASSERT(midpoint->edges.find(e01) == midpoint->edges.end());
    if (e02) DEBUG_ASSERT(midpoint->edges.find(e02) == midpoint->edges.end());
    if (e10) DEBUG_ASSERT(midpoint->edges.find(e10) == midpoint->edges.end());
    if (e11) DEBUG_ASSERT(midpoint->edges.find(e11) == midpoint->edges.end());
    if (e12) DEBUG_ASSERT(midpoint->edges.find(e12) == midpoint->edges.end());

    if (e00) DEBUG_ASSERT(oldpoint->edges.find(e00) == oldpoint->edges.end());
    if (e01) DEBUG_ASSERT(oldpoint->edges.find(e01) == oldpoint->edges.end());
    if (e02) DEBUG_ASSERT(oldpoint->edges.find(e02) == oldpoint->edges.end());
    if (e10) DEBUG_ASSERT(oldpoint->edges.find(e10) == oldpoint->edges.end());
    if (e11) DEBUG_ASSERT(oldpoint->edges.find(e11) == oldpoint->edges.end());
    if (e12) DEBUG_ASSERT(oldpoint->edges.find(e12) == oldpoint->edges.end());

    if (vA && e00) DEBUG_ASSERT(vA->edges.find(e00) == vA->edges.end());
    if (vA && e01) DEBUG_ASSERT(vA->edges.find(e01) == vA->edges.end());
    if (vA && e02) DEBUG_ASSERT(vA->edges.find(e02) == vA->edges.end());
    if (vA && e10) DEBUG_ASSERT(vA->edges.find(e10) == vA->edges.end());
    if (vA && e11) DEBUG_ASSERT(vA->edges.find(e11) == vA->edges.end());
    if (vA && e12) DEBUG_ASSERT(vA->edges.find(e12) == vA->edges.end());

    if (vB && e00) DEBUG_ASSERT(vB->edges.find(e00) == vB->edges.end());
    if (vB && e01) DEBUG_ASSERT(vB->edges.find(e01) == vB->edges.end());
    if (vB && e02) DEBUG_ASSERT(vB->edges.find(e02) == vB->edges.end());
    if (vB && e10) DEBUG_ASSERT(vB->edges.find(e10) == vB->edges.end());
    if (vB && e11) DEBUG_ASSERT(vB->edges.find(e11) == vB->edges.end());
    if (vB && e12) DEBUG_ASSERT(vB->edges.find(e12) == vB->edges.end());

    // clean up isolated verts
            delete_mp = (midpoint->edges.size() == 0);
    if (vA) delete_va = (vA->edges.size() == 0);
    if (vB) delete_vb = (vB->edges.size() == 0);

    // remove geom from f/v/e sets
    if (f0) faces.erase(f0);
    if (f1) faces.erase(f1);

    if (e00) { hedges.erase(e00); if (g_qem) queue.erase(e00->handle); }
    if (e01) { hedges.erase(e01); if (g_qem) queue.erase(e01->handle); }
    if (e02) { hedges.erase(e02); if (g_qem) queue.erase(e02->handle); }
    if (e10) { hedges.erase(e10); if (g_qem) queue.erase(e10->handle); }
    if (e11) { hedges.erase(e11); if (g_qem) queue.erase(e11->handle); }
    if (e12) { hedges.erase(e12); if (g_qem) queue.erase(e12->handle); }

                   vertices.erase(oldpoint);
    if (delete_mp) vertices.erase(midpoint);
    if (delete_va) vertices.erase(vA);
    if (delete_vb) vertices.erase(vB);

    /* collapse fins */
    if (e02->pair && e02->pair->IsDegenerate()) {
        printf("degen vA\n");
        vec3 &p = e02->pair->v->dstval;
        vec4 newloc = vec4(p.x, p.y, p.z, 1.0);
        state->degenA = this->Collapse(e02->pair, newloc);
    }
    if (e12 && e12->pair && e12->pair->IsDegenerate()) {
        printf("degen vB\n");
        vec3 &p = e12->pair->v->dstval;
        vec4 newloc = vec4(p.x, p.y, p.z, 1.0);
        state->degenB = this->Collapse(e12->pair, newloc);
    }

    /* update quadrics */
    midpoint->UpdateQ();
    foreach(Vertex* neighbor, midpoint->Vertices())
      neighbor->UpdateQ();

    /* rebalance heap */
    foreach(Hedge* h, midpoint->edges) {
        queue.update(h->handle, h);
        queue.update(h->next->handle, h->next);
        queue.update(h->next->next->handle, h->next->next);
    }

    DEBUG_ASSERT( this->check() );

    return state;
}

set<Vertex*>
Vertex::Vertices() {
    Hedge *e;
    set<Vertex*> neighbors;

    foreach(Hedge* h, edges) {
        assert(h->v == this);
        neighbors.insert(h->oppv());
        neighbors.insert(h->prev()->v);
    }

    return neighbors;
}

void
VertexSplit::Apply(Object* o) {
    /* recreate fins in reverse order */
    if (degenB) degenB->Apply(o);
    if (degenA) degenA->Apply(o);

    /* move target to original location */
    target->MoveTo(target_loc);

    /* fix up pairs */
    if (e01) e01->SetPair(e01->pair);
    if (e02) e02->SetPair(e02->pair);
    if (e11) e11->SetPair(e11->pair);
    if (e12) e12->SetPair(e12->pair);

    if (e00) DEBUG_ASSERT(e00->pair == e10);
    if (e10) DEBUG_ASSERT(e10->pair == e00);

    /* fix hedge->vertex pointers */
    foreach(Hedge* h, newpointHedges)
        newpoint->PullHedge(h);

    foreach(Hedge* h, targetHedges)
        target->PullHedge(h);

    if (vA) vA->PullHedge(e02);
    if (vB) vB->PullHedge(e12);

    /* register primitives with Object */
    if (e00) { o->hedges.insert(e00); if (g_qem) e00->handle = o->queue.push(e00); }
    if (e01) { o->hedges.insert(e01); if (g_qem) e01->handle = o->queue.push(e01); }
    if (e02) { o->hedges.insert(e02); if (g_qem) e02->handle = o->queue.push(e02); }
    if (e10) { o->hedges.insert(e10); if (g_qem) e10->handle = o->queue.push(e10); }
    if (e11) { o->hedges.insert(e11); if (g_qem) e11->handle = o->queue.push(e11); }
    if (e12) { o->hedges.insert(e12); if (g_qem) e12->handle = o->queue.push(e12); }

    o->faces.insert(f0);
    if (f1) o->faces.insert(f1);

    o->vertices.insert(target);
    o->vertices.insert(newpoint);
    if (vA) o->vertices.insert(vA);
    if (vB) o->vertices.insert(vB);

    /* make new vertices enter smoothly */
    newpoint->MoveFrom(target->Position());

    DEBUG_ASSERT( o->check() );
}

VertexSplit::VertexSplit(Hedge *e00)
    : e00(e00), degenA(NULL), degenB(NULL) {
    DEBUG_ASSERT(e00);

    e01 = e00->next;
    e02 = e00->prev();

    e10 = e00->pair;
    e11 = (e10) ? e10->next : NULL;
    e12 = (e10) ? e10->prev() : NULL;
    if (e10) DEBUG_ASSERT(e10->pair == e00);

    f0 = e00->f;
    f1 = (e10) ? e10->f: NULL;

    target = e00->v;
    target_loc = target->dstval;
    newpoint = e00->oppv();

    targetHedges = target->edges;
    newpointHedges = newpoint->edges;

    vA = (e02) ? e02->v : NULL;
    vB = (e12) ? e12->v : NULL;
}

bool
Hedge::IsDegenerate() {
    /*
     * A hedge is degenerate if it borders two coplanar faces
     */

    /* Not degen if this is a boundary hedge */
    if (this->pair == NULL)
        return false;

    assert(v == this->pair->oppv());
    assert(oppv() == this->pair->v);

    /* check if other vertex in each face is in same location */
    Vertex *v0 = this->prev()->v,
           *v1 = this->pair->prev()->v;

    //DEBUG_ASSERT(v0 != v1);
    return v0->dstval == v1->dstval;;
}

void
Object::Pop(bool many) {
    int npops = (many) ? std::min(100, (int) (0.1f * (float) faces.size())) : 1;
    npops = std::min(npops, (int) faces.size() - 2);
    for(int i = 0; i < npops; i++)
        vsplits.push_back(this->CollapseNext());
}

void
Object::Split(bool many) {
    int nsplits = (many) ? std::min(100, (int) (0.1f * (float) vsplits.size())) : 1;
    nsplits = std::min(nsplits, (int) vsplits.size());
    for(int i = 0; i < nsplits; i++) {
        vsplits.back()->Apply(this);
        delete vsplits.back();
        vsplits.pop_back();
    }
}

void
Vertex::MoveTo(vec4 p) {
    MoveTo( vec3(p.x, p.y, p.z) );
}

void
Vertex::MoveTo(vec3 dval) {
    srcval = Position();
    dstval = dval;
    framesleft = N_FRAMES_PER_SPLIT;
}

glm::vec3
Vertex::Position() {
    return dstval + (srcval-dstval) * vec3((float)framesleft/ (float)N_FRAMES_PER_SPLIT);
}

void
Vertex::MoveFrom(vec3 sval) {
    srcval = sval;
    framesleft = N_FRAMES_PER_SPLIT;
}


void
Vertex::UpdateQ() {
    Q = mat4(0.0);
    foreach(Hedge* h, edges) {
        Face* face = h->f;
        vec3 norm = face->Normal();
        vec3 pos = dstval;
        vec3 dv = pos * norm; // element-wise
        vec4 p(norm.x, norm.y, norm.z, 0.0 - dv.x - dv.y - dv.z); /* = [a b c d] */
        Q += outerProduct(p, p);
    }
}

bool
QEMCompare::operator() (Hedge *x, Hedge* y) const {
    /* min-cost hedge should be on top, but heap is a max heap so this is
     * the opposite of what you'd expect */
    double x_error = x->GetError(),
           y_error = y->GetError();

    /* reject nans */
    if (g_qem) {
        assert(x_error == x_error);
        assert(y_error == y_error);
    }

    return x_error > y_error;
}

double
Hedge::GetError() {
    mat4 Q = GetQ();
    vec4 v_bar = GetVBar();
    vec4 Qdotv = Q * v_bar;
    return dot(v_bar, Qdotv);
}

mat4
Vertex::GetQ() {
    UpdateQ(); // XXX could cache it
    return Q;
}
mat4
Hedge::GetQ() {
    return v->GetQ() + oppv()->GetQ();
}

vec4
Hedge::GetVBar() {
    mat4 Q = GetQ();
    Q[0][3] = 0.0;
    Q[1][3] = 0.0;
    Q[2][3] = 0.0;
    Q[3][3] = 1.0;

    if (g_qem and determinant(Q) != 0.0)
        return glm::column(inverse(Q), 3);
    else
        return homogenize( GetMidpoint() );
}

vec3
Hedge::GetMidpoint() {
    return vec3(0.5) * (v->dstval + oppv()->dstval);
}

Hedge*
Vertex::edge() {
    return *(edges.begin());
}

void
Vertex::PullHedge(Hedge* newInEdge, Vertex *old) {
    if (old)
        DEBUG_ASSERT(newInEdge->v == old);

    newInEdge->v->edges.erase(newInEdge);
    newInEdge->v = this;
    newInEdge->v->edges.insert(newInEdge);

    if ((long int) newInEdge < 100)
        assert(false);
}
