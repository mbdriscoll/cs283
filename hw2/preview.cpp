#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>

#include "scene.h"

using namespace glm;

Scene *scene = NULL;

void
Object::Render() {
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, &material.ambient.r);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, &material.diffuse.r);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, &material.specular.r);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, &material.emission.r);
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
    glutSolidSphere(r, 64, 64);
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
Light::Render() {
    printf("render light %d\n", lnum);
}

void
Light::Init() {
    GLenum n = GL_LIGHT0 + lnum;
    glEnable(n);

    glLoadMatrixf( (const float *) &xform[0] );
    glLightfv(n, GL_POSITION, &pos[0]);

    glLightfv(n, GL_AMBIENT, &material.ambient[0]);
    glLightfv(n, GL_DIFFUSE, &material.diffuse[0]);
    glLightfv(n, GL_SPECULAR, &material.specular[0]);


    glLightf(n, GL_CONSTANT_ATTENUATION,  material.atten[0]);
    glLightf(n, GL_LINEAR_ATTENUATION,    material.atten[1]);
    glLightf(n, GL_QUADRATIC_ATTENUATION, material.atten[2]);
}

void
Scene::Render() {
    foreach ( Light *light, lights )
        light->Render();

    foreach ( Object* obj, objs )
        obj->Render();
}

void initGL(void) {
    glClearColor (0.0, 0.0, 0.0, 0.0);
    glEnable(GL_DEPTH_TEST);
    glShadeModel (GL_SMOOTH);
    glEnable(GL_LIGHTING);
}

void display(void) {
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity ();

    scene->Render();

    glutSwapBuffers();
}

void reshape (int w, int h)
{
    glViewport (0, 0, (GLsizei) w, (GLsizei) h);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    gluPerspective(60.0, (GLfloat) w/(GLfloat) h, 1.0, 20.0);
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
