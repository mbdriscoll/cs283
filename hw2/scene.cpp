#include "scene.h"

#include <cstdio>
#include <cstdlib>

#define MAX_LINE_LENGTH 1024

using namespace std;

Scene::Scene(char *scenefilename) : output_fname("scene.png") {
    FILE* sfile = fopen(scenefilename, "r");
    if (sfile == NULL) {
        fprintf(stderr, "Unable to open scene file: %s\n", scenefilename);
        exit(2);
    }

    int nscanned;
    bool done = false;
    char *buf = (char*) malloc(MAX_LINE_LENGTH);

    MatSpec mat;
    std::stack<glm::mat4> xforms;
    xforms.push( glm::mat4(1.0) );

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
            fscanf(sfile, "%f %f %f %f %f %f %f %f %f %f",
                    &eye.x, &eye.y, &eye.z,
                    &center.x, &center.y, &center.z,
                    &up.x, &up.y, &up.z, &fov);

        } else if (cmd == "sphere") {
            Sphere *o = new Sphere(mat);
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
            Tri *t = new Tri(mat, verts[i0], verts[i1], verts[i2]);
            objs.push_back(t);

        } else if (cmd == "trinormal") {
            int i0, i1, i2;
            fscanf(sfile, "%d %d %d", &i0, &i1, &i2);
            TriNormal *t = new TriNormal(mat, vertnorms[i0], vertnorms[i1], vertnorms[i2]);
            objs.push_back(t);

        } else if (cmd == "translate") {
            glm::vec3 v;
            fscanf(sfile, "%f %f %f", &v.x, &v.y, &v.z);
            xforms.top() = glm::translate(xforms.top(), v);

        } else if (cmd == "rotate") {
            glm::vec3 v;
            float angle;
            fscanf(sfile, "%f %f %f %f", &v.x, &v.y, &v.z, &angle);
            xforms.top() = glm::rotate(xforms.top(), angle, v);

        } else if (cmd == "scale") {
            glm::vec3 v;
            fscanf(sfile, "%f %f %f", &v.x, &v.y, &v.z);
            xforms.top() = glm::scale(xforms.top(), v);

        } else if (cmd == "pushTransform") {
            xforms.push( glm::mat4(1.0) );

        } else if (cmd == "popTransform") {
            xforms.pop();

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
                    &mat.atten.r, &mat.atten.g, &mat.atten.b);

        } else if (cmd == "ambient") {
            fscanf(sfile, "%f %f %f",
                    &mat.ambient.r, &mat.ambient.g, &mat.ambient.b);

        } else if (cmd == "diffuse") {
            fscanf(sfile, "%f %f %f", &mat.diffuse.r, &mat.diffuse.g, &mat.diffuse.b);

        } else if (cmd == "specular") {
            fscanf(sfile, "%f %f %f", &mat.specular.r, &mat.specular.g, &mat.specular.b);

        } else if (cmd == "shininess") {
            fscanf(sfile, "%f", &mat.shininess);

        } else if (cmd == "emission") {
            fscanf(sfile, "%f %f %f", &mat.emission.r, &mat.emission.g, &mat.emission.b);

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
