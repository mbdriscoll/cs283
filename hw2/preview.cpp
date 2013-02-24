#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>

#include "scene.h"

void initGL(void) {
    glClearColor (0.0, 0.0, 0.0, 0.0);
    glEnable(GL_DEPTH_TEST);
    glShadeModel (GL_SMOOTH);
}

void display(void) {
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity ();

    // We will draw things a little deeper into the screen
    glTranslatef(0,0,-6);
    // Lets do some rotations here
    // ypoz gets changed by the animation function and zpoz only by keyboard.
    // remember you can press 'y' and 'z' to play with these
    // The angle is in degrees (0..360)
    glRotatef(0,0,1,0);
    glRotatef(0,0,0,1);

    // I'm going to draw something simple here as we haven't yet discussed loading models
    glColor3f(1,0,0);
    glBegin(GL_TRIANGLES);
    glVertex3f(0,2,0);
    glVertex3f(3,0,0);
    glVertex3f(-3,0,0);
    glEnd();

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

    glutMainLoop();
}
