#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>

#include "scene.h"

using namespace glm;

Scene *scene = NULL;

void
Object::Render() {
    glColor3fv( &material.ambient.r );
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
    glPushMatrix();
    glTranslatef(p.x, p.y, p.z);
    glutSolidSphere(r, 64, 64);
    glPopMatrix();
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
}

void
Scene::Render() {
    foreach( Object* o, objs )
        o->Render();
}

void initGL(void) {
    glClearColor (0.0, 0.0, 0.0, 0.0);
    glEnable(GL_DEPTH_TEST);
    glShadeModel (GL_SMOOTH);
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
    int argc = 0;
    glutInit(&argc, NULL);
    glutInitDisplayMode (GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize (width, height);
    glutCreateWindow (output_fname.c_str());
    initGL();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);

    scene = this;

    glutMainLoop();
}
