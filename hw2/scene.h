#ifndef _TRACE_SCENE_H_
#define _TRACE_SCENE_H_

#include <string>
#include <vector>
#include <stack>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

typedef std::pair<glm::vec3,glm::vec3> vertnorm;

class MatSpec {
  public:
    glm::vec3 diffuse, specular, emission;
    float shininess;
};

class LightSpec {
  public:
    glm::vec3 atten, ambient;
};

class Object {
  public:
    Object(MatSpec &mat) : mat(mat) {}
    MatSpec mat;
};

class Sphere : public Object {
  public:
    Sphere(MatSpec &mat) : Object(mat) {}
    glm::vec3 p;
    float r;
};

class Tri : public Object {
  public:
    Tri(MatSpec &mat, glm::vec3 v0, glm::vec3 v1, glm::vec3 v2) :
        Object(mat), v0(v0), v1(v1), v2(v2) { }
    glm::vec3 v0, v1, v2;
};

class TriNormal : public Object {
  public:
    TriNormal(MatSpec &mat, vertnorm v0, vertnorm v1, vertnorm v2) :
        Object(mat), v0(v0), v1(v1), v2(v2) { }
    vertnorm v0, v1, v2;
};

class Light {
  public:
    Light(LightSpec &lspec) : lspec(lspec) {}
    glm::vec3 pos, color;
    LightSpec lspec;
};

class Scene {
  public:
    Scene(char *scenefilename);
    void RayTrace();
  private:
    int width, height, maxdepth;
    std::string output_fname;

    MatSpec mat;
    LightSpec lightspec;

    glm::vec3 camera_at, camera_to, camera_up;
    float fov;

    std::vector<glm::vec3> verts;
    std::vector<vertnorm> vertnorms;

    std::vector<Object*> objs;
    std::vector<Light*> lights;

    std::stack<glm::mat4> xforms;
};

#endif /* _TRACE_SCENE_H_ */
