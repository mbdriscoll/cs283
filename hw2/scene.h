#ifndef _TRACE_SCENE_H_
#define _TRACE_SCENE_H_

#include <string>
#include <vector>
#include <stack>

#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

typedef std::pair<glm::vec3,glm::vec3> vertnorm;

class MatSpec {
  public:
    MatSpec() :
        ambient( glm::vec3(0.2f, 0.2f, 0.2f) ),
        atten  ( glm::vec3(1.0f, 0.0f, 0.0f) )
    {}

    glm::vec3 atten, ambient, diffuse, specular, emission;
    float shininess;
};

class Object {
  public:
    Object(MatSpec &mat) : mat(mat) {}

    virtual void Render();
    MatSpec mat;
};

class Sphere : public Object {
  public:
    Sphere(MatSpec &mat) : Object(mat) {}
    virtual void Render();

    glm::vec3 p;
    float r;
};

class Tri : public Object {
  public:
    Tri(MatSpec &mat, glm::vec3 v0, glm::vec3 v1, glm::vec3 v2) :
        Object(mat), v0(v0), v1(v1), v2(v2) { }
    virtual void Render();

    glm::vec3 v0, v1, v2;
};

class TriNormal : public Object {
  public:
    TriNormal(MatSpec &mat, vertnorm v0, vertnorm v1, vertnorm v2) :
        Object(mat), v0(v0), v1(v1), v2(v2) { }
    void Render();

    vertnorm v0, v1, v2;
};

class Light {
  public:
    void Render();

    glm::vec3 pos, color;
};

class Scene {
  public:
    Scene(char *scenefilename);
    void RayTrace();
    void Preview();
    void Render();

  private:
    int width, height, maxdepth;
    std::string output_fname;

    glm::vec3 eye, center, up;
    float fov;

    std::vector<glm::vec3> verts;
    std::vector<vertnorm> vertnorms;

    std::vector<Object*> objs;
    std::vector<Light*> lights;
};

#endif /* _TRACE_SCENE_H_ */
