#include "scene.h"

#include <cstdio>
#include <cstdlib>

#define MAX_LINE_LENGTH 1024

using namespace std;

inline glm::mat4
XF(std::vector<glm::mat4> &xforms) {
    glm::mat4 xf(1.0);
    foreach (glm::mat4 &M, xforms)
        xf = xf * M;
    return xf;
}

Scene::Scene(char *scenefilename) : output_fname("scene.png") {
    FILE* sfile = fopen(scenefilename, "r");
    if (sfile == NULL) {
        fprintf(stderr, "Unable to open scene file: %s\n", scenefilename);
        exit(2);
    }

    int nscanned;
    bool done = false;
    char *buf = (char*) malloc(MAX_LINE_LENGTH);

    MatSpec material;
    std::vector<glm::mat4> xforms;
    xforms.reserve(20);
    xforms.push_back( glm::mat4(1.0) );

    while (!done) {
        nscanned = fscanf(sfile, "%s", buf);

        // -------------------------------------------------------------------
        // handle non-commands (empty lines, comments, eof, etc)

        if (nscanned == 0) {                  // blank line
            continue;
        } else if (nscanned == EOF) {         // end of file
            break;
        } else if (buf[0] == '#') {           // comment
            fscanf(sfile, "%255[^\n]", buf);
            continue;
        }

        // -------------------------------------------------------------------
        // handle commands

        string cmd(buf);
        if (cmd == "size") {
            fscanf(sfile, "%d %d", &width, &height);

        } else if (cmd == "maxdepth") {
            fscanf(sfile, "%d", &maxdepth);

        } else if (cmd == "output") {
            fscanf(sfile, "%s", buf);
            output_fname = string(buf);

        } else if (cmd == "camera") {
            glm::vec3 eye, center, up;
            fscanf(sfile, "%f %f %f %f %f %f %f %f %f %f",
                    &eye.x, &eye.y, &eye.z,
                    &center.x, &center.y, &center.z,
                    &up.x, &up.y, &up.z, &fov);
            xforms.push_back( glm::lookAt(eye,center,up) );

        } else if (cmd == "sphere") {
            Sphere *o = new Sphere(XF(xforms), material);
            fscanf(sfile, "%f %f %f %f", &o->p.x, &o->p.y, &o->p.z, &o->r);
            objs.push_back(o);

        } else if (cmd == "maxverts") {
            int maxverts;
            fscanf(sfile, "%d", &maxverts);
            verts.reserve(maxverts);

        } else if (cmd == "maxvertnorms") {
            int maxvertnorms;
            fscanf(sfile, "%d", &maxvertnorms);
            vertnorms.reserve(maxvertnorms);

        } else if (cmd == "vertex") {
            glm::vec3 v;
            fscanf(sfile, "%f %f %f", &v.x, &v.y, &v.z);
            verts.push_back(v);

        } else if (cmd == "vertexnormal") {
            glm::vec3 v, n;
            fscanf(sfile, "%f %f %f %f %f %f",
                    &v.x, &v.y, &v.z, &n.x, &n.y, &n.z);
            vertnorms.push_back(vertnorm(v,n));

        } else if (cmd == "tri") {
            int i0, i1, i2;
            fscanf(sfile, "%d %d %d", &i0, &i1, &i2);
            Tri *t = new Tri(XF(xforms), material, verts[i0], verts[i1], verts[i2]);
            objs.push_back(t);

        } else if (cmd == "trinormal") {
            int i0, i1, i2;
            fscanf(sfile, "%d %d %d", &i0, &i1, &i2);
            TriNormal *t = new TriNormal(XF(xforms), material,
                    vertnorms[i0], vertnorms[i1], vertnorms[i2]);
            objs.push_back(t);

        } else if (cmd == "translate") {
            glm::vec3 v;
            fscanf(sfile, "%f %f %f", &v.x, &v.y, &v.z);
            xforms.back() = glm::translate(xforms.back(), v);

        } else if (cmd == "rotate") {
            glm::vec3 v;
            float angle;
            fscanf(sfile, "%f %f %f %f", &v.x, &v.y, &v.z, &angle);
            xforms.back() = glm::rotate(xforms.back(), angle, v);

        } else if (cmd == "scale") {
            glm::vec3 v;
            fscanf(sfile, "%f %f %f", &v.x, &v.y, &v.z);
            xforms.back() = glm::scale(xforms.back(), v);

        } else if (cmd == "pushTransform") {
            xforms.push_back( glm::mat4(1.0) );

        } else if (cmd == "popTransform") {
            xforms.pop_back();

        } else if (cmd == "directional") {
            Light *l = new Light();
            fscanf(sfile, "%f %f %f %f %f %f",
                    &l->pos.x, &l->pos.y, &l->pos.z,
                    &l->color.r, &l->color.g, &l->color.b);
            lights.push_back(l);

        } else if (cmd == "point") {
            Light *l = new Light();
            fscanf(sfile, "%f %f %f %f %f %f",
                    &l->pos.x, &l->pos.y, &l->pos.z,
                    &l->color.r, &l->color.g, &l->color.b);

        } else if (cmd == "attentuation") {
            fscanf(sfile, "%f %f %f",
                    &material.atten.r, &material.atten.g, &material.atten.b);

        } else if (cmd == "ambient") {
            fscanf(sfile, "%f %f %f",
                    &material.ambient.r, &material.ambient.g, &material.ambient.b);

        } else if (cmd == "diffuse") {
            fscanf(sfile, "%f %f %f",
                    &material.diffuse.r, &material.diffuse.g, &material.diffuse.b);

        } else if (cmd == "specular") {
            fscanf(sfile, "%f %f %f",
                    &material.specular.r, &material.specular.g, &material.specular.b);

        } else if (cmd == "shininess") {
            fscanf(sfile, "%f", &material.shininess);

        } else if (cmd == "emission") {
            fscanf(sfile, "%f %f %f",
                    &material.emission.r, &material.emission.g, &material.emission.b);

        } else {
            printf("Unregcognized command: %s\n", cmd.c_str());
            exit(3);
        }
    }

    fclose(sfile);
}

void
Scene::RayTrace() {
    printf("raytracing...\n");
}
