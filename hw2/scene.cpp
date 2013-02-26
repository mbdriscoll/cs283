#include "scene.h"
#include "image.h"

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
            fscanf(sfile, "%f %f %f %f %f %f %f %f %f %f",
                    &eye.x, &eye.y, &eye.z,
                    &center.x, &center.y, &center.z,
                    &up.x, &up.y, &up.z, &fov);
            xforms.push_back( glm::lookAt(eye,center,up) );

        } else if (cmd == "sphere") {
            float r;
            glm::vec3 p;
            fscanf(sfile, "%f %f %f %f", &p.x, &p.y, &p.z, &r);
            glm::mat4 M = glm::translate(glm::mat4(1), p);
            Sphere *o = new Sphere(XF(xforms)*M, material, r);
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
            glm::vec4 pos(0), color(1);
            fscanf(sfile, "%f %f %f %f %f %f",
                    &pos.x, &pos.y, &pos.z,
                    &color.r, &color.g, &color.b);
            Light *l = new Light(XF(xforms), material, pos, color );
            lights.push_back(l);

        } else if (cmd == "point") {
            glm::vec4 pos(1), color(1);
            fscanf(sfile, "%f %f %f %f %f %f",
                    &pos.x, &pos.y, &pos.z,
                    &color.r, &color.g, &color.b);
            Light *l = new Light(XF(xforms), material, pos, color );
            lights.push_back(l);

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

glm::vec3
Scene::CastRay(glm::vec3 origin, glm::vec3 target) {
    return glm::vec3(1, 0, 0);
}

void
Scene::RayTrace() {
    printf("raytracing...\n");
    glm::vec3 *buffer = (glm::vec3*) malloc(width * height * sizeof(glm::vec3));

    float aspect = width / (float) height;
    float dj = glm::length(center-eye) * tan(0.5 * fov);
    float di = dj / aspect;
    glm::vec3 vj = dj * glm::normalize( glm::cross(center-eye, up) );
    glm::vec3 vi = di * glm::normalize( up );

    glm::vec3 ur = center + glm::vec3(1.00) * (vi + vj),
              ul = center + glm::vec3(1.00) * (vi - vj),
              lr = center + glm::vec3(1.00) * (-vi + vj),
              ll = center + glm::vec3(1.00) * (-vi - vj);
    float pix_width  = 2.0f * dj / (float) width;
    float pix_height = 2.0f * di / (float) height;

    for (int i = 0; i < height; i += 1) {
        for (int j = 0; j < width; j += 1) {
            glm::vec3 ulp = ul +
                glm::vec3(i/(float)height) * (lr-ur) +
                glm::vec3(j/(float)width ) * (ur-ul);
            glm::vec3 urp = ulp + pix_width * glm::normalize(vj),
                      llp = ulp - pix_height * glm::normalize(vi),
                      lrp = ulp + (urp-ulp) + (llp-ulp);
            glm::vec3 target = 0.25f * (ulp + urp + llp + lrp);

            buffer[i*width+j] = CastRay(eye, target);
        }
    }

    writeImage((char*)(output_fname+".png").c_str(), width, height, buffer, (char*)"Image");
}
