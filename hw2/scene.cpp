#include "scene.h"

#include <cstdio>
#include <cstdlib>

#define MAX_LINE_LENGTH 1024

using namespace std;

Scene::Scene(char *scenefilename) {
    FILE* sfile = fopen(scenefilename, "r");
    if (sfile == NULL) {
        fprintf(stderr, "Unable to open scene file: %s\n", scenefilename);
        exit(2);
    }

    int nscanned;
    bool done = false;
    char *buf = (char*) malloc(MAX_LINE_LENGTH);
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
                    &camera_at.x, &camera_at.y, &camera_at.z,
                    &camera_to.x, &camera_to.y, &camera_to.z,
                    &camera_up.x, &camera_up.y, &camera_up.z, &fov);

        } else if (cmd == "sphere") {
            Sphere *o = new Sphere();
            fscanf(sfile, "%f %f %f %f", &o->p.x, &o->p.y, &o->p.z, &o->r);
            // TODO register sphere with scene

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
            Tri *t = new Tri(verts[i0], verts[i1], verts[i2]);
            // TODO register triangle with scene

        } else if (cmd == "trinormal") {
            int i0, i1, i2;
            fscanf(sfile, "%d %d %d", &i0, &i1, &i2);
            TriNormal *t = new TriNormal(vertnorms[i0], vertnorms[i1], vertnorms[i2]);
            // TODO register trinormal with scene

        } else if (cmd == "translate") {
            glm::vec3 v;
            fscanf(sfile, "%f %f %f", &v.x, &v.y, &v.z);

        } else if (cmd == "rotate") {
            glm::vec3 v;
            float angle;
            fscanf(sfile, "%f %f %f %f", &v.x, &v.y, &v.z, &angle);

        } else if (cmd == "scale") {
            glm::vec3 v;
            fscanf(sfile, "%f %f %f", &v.x, &v.y, &v.z);

        } else if (cmd == "pushTransform") {

        } else if (cmd == "popTransform") {

        } else if (cmd == "directional") {
            Light *l = new Light();
            fscanf(sfile, "%f %f %f %f %f %f",
                    &l->pos.x, &l->pos.y, &l->pos.z,
                    &l->color.r, &l->color.g, &l->color.b);
            // TODO register light with scene

        } else if (cmd == "point") {
            Light *l = new Light();
            fscanf(sfile, "%f %f %f %f %f %f",
                    &l->pos.x, &l->pos.y, &l->pos.z,
                    &l->color.r, &l->color.g, &l->color.b);

        } else if (cmd == "attentuation") {
            fscanf(sfile, "%f %f %f", &atten.r, &atten.g, &atten.b);

        } else if (cmd == "ambient") {
            fscanf(sfile, "%f %f %f", &ambient.r, &ambient.g, &ambient.b);

        } else if (cmd == "diffuse") {
            fscanf(sfile, "%f %f %f", &diffuse.r, &diffuse.g, &diffuse.b);

        } else if (cmd == "specular") {
            fscanf(sfile, "%f %f %f", &specular.r, &specular.g, &specular.b);

        } else if (cmd == "shininess") {
            fscanf(sfile, "%f", &shininess);

        } else if (cmd == "emission") {
            fscanf(sfile, "%f %f %f", &emission.r, &emission.g, &emission.b);
        } else {
            printf("Unregcognized command: %s\n", cmd.c_str());
            exit(3);
        }
    }

    fclose(sfile);
}

void
Scene::RayTrace() {
}
