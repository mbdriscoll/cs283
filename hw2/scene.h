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
    Object(glm::mat4 xform, MatSpec &material) :
        xform(xform), material(material)
    {}

    virtual void Render();

    MatSpec material;
    glm::mat4 xform;
};

class Sphere : public Object {
  public:
    Sphere(glm::mat4 xform, MatSpec &material) :
        Object(xform, material)
    {}
    virtual void Render();

    glm::vec3 p;
    float r;
};

class Tri : public Object {
  public:
    Tri(glm::mat4 xform, MatSpec &material, glm::vec3 v0, glm::vec3 v1, glm::vec3 v2) :
        Object(xform, material), v0(v0), v1(v1), v2(v2) { }
    virtual void Render();

    glm::vec3 v0, v1, v2;
};

class TriNormal : public Object {
  public:
    TriNormal(glm::mat4 xform, MatSpec &material, vertnorm vn0, vertnorm vn1, vertnorm vn2) :
        Object(xform, material), vn0(vn0), vn1(vn1), vn2(vn2) { }
    void Render();

    vertnorm vn0, vn1, vn2;
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

    float fov;

    std::vector<glm::vec3> verts;
    std::vector<vertnorm> vertnorms;

    std::vector<Object*> objs;
    std::vector<Light*> lights;
};

#endif /* _TRACE_SCENE_H_ */
