#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>

#include "scene.h"

using namespace glm;

Scene *scene = NULL;

void
Object::Render() {
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, &material.ambient[0]);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, &material.ambient[0]);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, &material.diffuse[0]);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, &material.specular[0]);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, &material.emission[0]);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &material.shininess);

    glLoadMatrixf( (const float *) &xform[0] );
}

void
Tri::Render() {
    this->Object::Render();
    vec3 norm = normalize( cross(v1-v0,v2-v1) );
    glBegin(GL_TRIANGLES);
    glNormal3fv( &norm.x );
    glVertex3fv( &v0.x );
    glVertex3fv( &v1.x );
    glVertex3fv( &v2.x );
    glEnd();
}

void
Sphere::Render() {
    this->Object::Render();
    glutSolidSphere(r, 32, 32);
}

void
TriNormal::Render() {
    this->Object::Render();
    glBegin(GL_TRIANGLES);

    glNormal3fv( &vn0.second.x );
    glVertex3fv( &vn0.first.x );

    glNormal3fv( &vn1.second.x );
    glVertex3fv( &vn1.first.x );

    glNormal3fv( &vn2.second.x );
    glVertex3fv( &vn2.first.x );

    glEnd();
}

void
Light::Init() {
    glLoadMatrixf( (const float *) &xform[0] );

    GLenum n = GL_LIGHT0 + lnum;

    glEnable(n);
    glLightfv(n, GL_AMBIENT,  &material.ambient[0]);
    glLightfv(n, GL_POSITION, &pos[0]);

    glLightf(n, GL_CONSTANT_ATTENUATION,  material.atten[0]);
    glLightf(n, GL_LINEAR_ATTENUATION,    material.atten[1]);
    glLightf(n, GL_QUADRATIC_ATTENUATION, material.atten[2]);
}

void
Scene::Render() {
    foreach ( Object* obj, objs )
        obj->Render();

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

    glDisable(GL_LIGHTING);
    glPolygonMode(GL_FRONT, GL_LINE);

    glLoadIdentity();
    gluLookAt(eye.x, eye.y, eye.z,
            center.x, center.y, center.z,
            up.x, up.y, up.z);
    glPointSize(0.1f);

    for (int i = 0; i < height; i += 1) {
        for (int j = 0; j < width; j += 1) {
            glm::vec3 ulp = ul +
                glm::vec3(i/(float)height) * (lr-ur) +
                glm::vec3(j/(float)width ) * (ur-ul);
            glm::vec3 urp = ulp + pix_width * normalize(vj),
                      llp = ulp - pix_height * normalize(vi),
                      lrp = ulp + (urp-ulp) + (llp-ulp);
            glm::vec3 center = 0.25f * (ulp + urp + llp + lrp);

            glColor3f(0,0,0.5);
            glBegin(GL_QUADS);
            glVertex3fv(&ulp[0]);
            glVertex3fv(&llp[0]);
            glVertex3fv(&lrp[0]);
            glVertex3fv(&urp[0]);
            glEnd();

            glColor3f(1,1,1);
            glBegin(GL_POINTS);
            glVertex3fv(&center[0]);
            glEnd();
        }
    }

    glEnable(GL_LIGHTING);
    glPolygonMode(GL_FRONT, GL_FILL);
}

void initGL(void) {
    glClearColor(0,0,0,0); // bg color
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_LIGHTING);
    glShadeModel(GL_FLAT);
}

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    scene->Render();

    glutSwapBuffers();
}

void reshape (int w, int h)
{
    glViewport (0, 0, (GLsizei) w, (GLsizei) h);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    gluPerspective(scene->fov, (GLfloat) w/(GLfloat) h, 0.5, 100.0);
    glMatrixMode (GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 'q': // ESC key
            exit(0);
            break;
    }
}

void
Scene::Preview() {
    scene = this;

    int argc = 0;
    glutInit(&argc, NULL);
    glutInitDisplayMode (GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize (width, height);
    glutCreateWindow (output_fname.c_str());

    initGL();

    foreach (Light *light, lights)
        light->Init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);

    glutMainLoop();
}
