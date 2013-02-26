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
        ambient  ( glm::vec4(0.2f, 0.2f, 0.2f, 0.0f) ),
        atten    ( glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) ),
        diffuse  ( glm::vec4(0,0,0,1) ),
        specular ( glm::vec4(0,0,0,1) ),
        emission ( glm::vec4(0,0,0,1) )
    {}

    glm::vec4 atten, ambient, diffuse, specular, emission;
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
    Sphere(glm::mat4 xform, MatSpec &material, float r) :
        Object(xform, material), r(r)
    {}
    virtual void Render();

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

static int light_next_num = 0;
class Light {
  public:
    Light(glm::mat4 xform, MatSpec &material, glm::vec4 pos, glm::vec4 color) :
        pos(pos), material(material), xform(xform), color(color), lnum(light_next_num++)
    {}
    void Init();

    MatSpec material;
    glm::vec4 pos, color;
    glm::mat4 xform;
    int lnum;
};

class Scene {
  public:
    Scene(char *scenefilename);
    void RayTrace();
    void Preview();
    void Render();

    float fov;
    int width, height, maxdepth;

  private:
    std::string output_fname;

    std::vector<glm::vec3> verts;
    std::vector<vertnorm> vertnorms;

    std::vector<Object*> objs;
    std::vector<Light*> lights;
};

#endif /* _TRACE_SCENE_H_ */
