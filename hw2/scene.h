#ifndef _TRACE_SCENE_H_
#define _TRACE_SCENE_H_

#include <string>
#include <vector>

#include <glm/glm.hpp>

typedef std::pair<glm::vec3,glm::vec3> vertnorm;

class Scene {
  public:
    Scene(char *scenefilename);
    void RayTrace();
  private:
    int width, height, maxdepth;
    std::string output_fname;
    glm::vec3 atten, ambient, diffuse, specular, emission;
    float shininess;

    glm::vec3 camera_at, camera_to, camera_up;
    float fov;
    std::vector<glm::vec3> verts;
    std::vector<vertnorm> vertnorms;
};

class Sphere {
  public:
    glm::vec3 p;
    float r;
};

class Tri {
  public:
    Tri(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2) :
        v0(v0), v1(v1), v2(v2) { }
    glm::vec3 v0, v1, v2;
};

class TriNormal {
  public:
    TriNormal(vertnorm v0, vertnorm v1, vertnorm v2) :
        v0(v0), v1(v1), v2(v2) { }
    vertnorm v0, v1, v2;
};

class Light {
  public:
    glm::vec3 pos, color;
};

#endif /* _TRACE_SCENE_H_ */
