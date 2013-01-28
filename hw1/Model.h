#include <cstdio>
#include <vector>

class Model {
  public:
      Model(FILE* input);
      void Render();
      void SetCenterSize(float *center, float *size);

      class Vertex {
          Model *model;
        public:
          float x,y,z;
          Vertex(Model *model, float x, float y, float z) : model(model), x(x), y(y), z(z) { }
          void Render();
      };

      class Face {
          Model *model;
          int v0, v1, v2;
        public:
          Face(Model *model, int v0, int v1, int v2) : model(model), v0(v0), v1(v1), v2(v2) { }
          void Render();
      };

  protected:
      std::vector<Vertex> vertices;
      std::vector<Face> faces;
};
