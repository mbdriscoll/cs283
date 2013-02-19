#include "viewer.h"

#include <stdlib.h>
#include <cfloat>
#include <vector>
#include <fstream>
#include <sstream>

#include "Object.h"
#include "extra/gl_hud.h"

Object *g_model = NULL;

int   g_frame = 0,
      g_animate = 0,
      g_animateDirection = 0, /* 1: do splits   0: do pops  */
      g_repeatCount = 0;

// GUI variables
int   g_fullscreen=0,
      g_freeze = 0,
      g_wire = 0,
      g_drawVertexNormals = 0,
      g_drawFaceNormals = 0,
      g_mbutton[3] = {0, 0, 0};

int   g_displayPatchColor = 1;

float g_rotate[2] = {0, 0},
      g_prev_x = 0,
      g_prev_y = 0,
      g_dolly = 5,
      g_pan[2] = {0, 0},
      g_center[3] = {0, 0, 0},
      g_size = 0;

int   g_width = 1024,
      g_height = 1024;

int   g_qem = 1;

GLhud g_hud;

#if 0
GLuint g_transformUB = 0,
       g_transformBinding = 0,
       g_tessellationUB = 0,
       g_tessellationBinding = 0,
       g_lightingUB = 0,
       g_lightingBinding = 0;
GLuint g_primQuery = 0;
#endif

std::vector<int> g_coarseEdges;
std::vector<float> g_coarseEdgeSharpness;
std::vector<float> g_coarseVertexSharpness;

static void
checkGLErrors(std::string const & where = "")
{
    GLuint err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        /*
        std::cerr << "GL error: "
                  << (where.empty() ? "" : where + " ")
                  << err << "\n";
        */
    }
}

//------------------------------------------------------------------------------
static void
fitFrame() {

    g_pan[0] = g_pan[1] = 0;
    g_dolly = g_size;
}

//------------------------------------------------------------------------------
static void
initializeShape(const char* input_filename) {
    printf("Reading object in file %s.\n", input_filename);

    FILE* input_file = fopen(input_filename, "r");
    if (input_file == NULL) {
        printf("Could not open model at %s.\n", input_filename);
        exit(2);
    }

    g_model = new Object(input_file);
    g_model->SetCenterSize((float*) &g_center, &g_size);

    fclose(input_file);

    fitFrame();
}

//------------------------------------------------------------------------------
static void
display() {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, g_width, g_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glColor3f(1, 1, 1);
    glBegin(GL_QUADS);
    glColor3f(0.5f, 0.5f, 0.5f);
    glVertex3f(-1, -1, 1);
    glVertex3f( 1, -1, 1);
    glColor3f(0, 0, 0);
    glVertex3f( 1,  1, 1);
    glVertex3f(-1,  1, 1);
    glEnd();

    double aspect = g_width/(double)g_height;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, aspect, 0.01, 500.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(-g_pan[0], -g_pan[1], -g_dolly);
    glRotatef(g_rotate[1], 1, 0, 0);
    glRotatef(g_rotate[0], 0, 1, 0);
    glRotatef(-90, 1, 0, 0); // z-up model

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof (GLfloat) * 6, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof (GLfloat) * 6, (float*)12);

    //g_model->DrawPoints();
    g_model->DrawNormals(g_drawVertexNormals, g_drawFaceNormals);

    glEnable(GL_LIGHTING);
    glShadeModel(GL_SMOOTH);

    glColor3f(0.5f, 0.2f, 0.7f);
    GLfloat materialShininess[] = {128.0f};
    GLfloat materialAmbDiff[] = {0.9f, 0.1f, 0.1f, 1.0f};
    GLfloat materialSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, materialAmbDiff);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpecular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, materialShininess);

    glPolygonMode(GL_FRONT_AND_BACK, (g_wire == 0) ? GL_LINE : GL_FILL);
    g_model->Render();

    //glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    //glDisableClientState(GL_VERTEX_ARRAY);
    glFinish();

    if (g_hud.IsVisible()) {
        g_hud.DrawString(10, -40,  "Vertices:   %d/%d", g_model->vertices.size(),
                g_model->vertices.size() + g_model->vsplits.size());
        g_hud.DrawString(10, -20,  "Faces:      %d", g_model->faces.size());
    }

    g_hud.Flush();

    glFinish();
    glutSwapBuffers();
    glFinish();

    checkGLErrors("display leave");
}

//------------------------------------------------------------------------------
static void
motion(int x, int y) {

    if (g_mbutton[0] && !g_mbutton[1] && !g_mbutton[2]) {
        // orbit
        g_rotate[0] += x - g_prev_x;
        g_rotate[1] += y - g_prev_y;
    } else if (!g_mbutton[0] && g_mbutton[1] && !g_mbutton[2]) {
        // pan
        g_pan[0] -= g_dolly*(x - g_prev_x)/g_width;
        g_pan[1] += g_dolly*(y - g_prev_y)/g_height;
    } else if ((g_mbutton[0] && g_mbutton[1] && !g_mbutton[2]) or
               (!g_mbutton[0] && !g_mbutton[1] && g_mbutton[2])) {
        // dolly
        g_dolly -= g_dolly*0.01f*(x - g_prev_x);
        if(g_dolly <= 0.01) g_dolly = 0.01f;
    }

    g_prev_x = float(x);
    g_prev_y = float(y);
}

//------------------------------------------------------------------------------
static void
mouse(int button, int state, int x, int y) {

    if (button == 0 && state == 1 && g_hud.MouseClick(x, y)) return;

    if (button < 3) {
        g_prev_x = float(x);
        g_prev_y = float(y);
        g_mbutton[button] = !state;
    }
}

//------------------------------------------------------------------------------
static void
quit() {
    //glDeleteQueries(1, &g_primQuery);
    exit(0);
}

//------------------------------------------------------------------------------
static void
reshape(int width, int height) {
    g_width = width;
    g_height = height;
    g_hud.Rebuild(width, height);
}

//------------------------------------------------------------------------------
static void toggleFullScreen() {

    static int x,y,w,h;

    g_fullscreen = !g_fullscreen;

    if (g_fullscreen) {
        x = glutGet((GLenum)GLUT_WINDOW_X);
        y = glutGet((GLenum)GLUT_WINDOW_Y);
        w = glutGet((GLenum)GLUT_WINDOW_WIDTH);
        h = glutGet((GLenum)GLUT_WINDOW_HEIGHT);

        glutFullScreen( );

        reshape( glutGet(GLUT_SCREEN_WIDTH),
                 glutGet(GLUT_SCREEN_HEIGHT) );
    } else {
        glutReshapeWindow(w, h);
        glutPositionWindow(x,y);
        reshape( w, h );
    }
}

//------------------------------------------------------------------------------
static void
keyboard(unsigned char key, int x, int y) {

    if (g_hud.KeyDown(key)) return;

    switch (key) {
        case 'q': quit();
        case 'f': fitFrame(); break;
        case '\t': toggleFullScreen(); break;
        case 0x1b: g_hud.SetVisible(!g_hud.IsVisible()); break;

        /* edge pops */
        case '-':
        case '_': g_model->Pop(/* many = */ key == '_'); break;

        /* vertex splits */
        case '=':
        case '+': g_model->Split(/* many = */ key == '+'); break;
    }
}

//------------------------------------------------------------------------------
static void
callbackWireframe(int b)
{
    g_wire = b;
}

static void
callbackDisplayVertexNormal(bool checked, int n)
{
    g_drawVertexNormals = checked;
}

static void
callbackDisplayFaceNormal(bool checked, int n)
{
    g_drawFaceNormals = checked;
}

static void
callbackFreeze(bool checked, int f)
{
    g_freeze = checked;
}

static void
callbackQEM(bool checked, int n) {
    g_qem = checked;
}

static void
callbackAnimate(bool checked, int n) {
    g_animate = checked;
}

static void
initHUD()
{
    g_hud.Init(g_width, g_height);

    g_hud.AddCheckBox("Show face normals (e)", false, 10, 10, callbackDisplayFaceNormal, 0, 'e');
    g_hud.AddCheckBox("Show vertex (v)",       false, 10, 30, callbackDisplayVertexNormal, 0, 'v');

    g_hud.AddRadioButton(1, "Wire (w)",    g_wire == 0, 10, 60, callbackWireframe, 0, 'w');
    g_hud.AddRadioButton(1, "Shaded",      g_wire == 1, 10, 80, callbackWireframe, 1, 'w');

    g_hud.AddCheckBox("Quadrics (m)", true, 10, 110, callbackQEM, 0, 'm');
    g_hud.AddCheckBox("Animate (a)", 0, 10, 130, callbackAnimate, 0, 'a');
}

//------------------------------------------------------------------------------
static void
initGL()
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glEnable(GL_LIGHT0);
    glColor3f(1, 1, 1);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    GLfloat color[4] = {1, 1, 1, 1};
    GLfloat position[4] = {5, 5, 10, 1};
    GLfloat ambient[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    GLfloat diffuse[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat shininess = 25.0;

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, color);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, color);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &shininess);
    glLightfv(GL_LIGHT0, GL_POSITION, position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
}

//------------------------------------------------------------------------------
static void
idle() {

    if (not g_freeze)
        g_frame++;

    bool doneAnimating = g_model->Render();

    /* 0: do nothing.   1: do splits   -1: do pops  */
    if (doneAnimating && g_animate) {
        if (g_model->vsplits.size() == 0) {
            printf("Doing edge pops\n");
            g_animateDirection = 0;
        } else if (g_model->queue.size() <= 16) {
            printf("Doing vertex splits.\n");
            g_animateDirection = 1;
        }

        if (g_animateDirection)
            g_model->Split();
        else
            g_model->Pop();
    }

    glutPostRedisplay();

    if (g_repeatCount != 0 and g_frame >= g_repeatCount)
        quit();
}

//------------------------------------------------------------------------------
int main(int argc, char ** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA |GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(g_width, g_height);
    glutCreateWindow("CS283 Viewer");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutKeyboardFunc(keyboard);
    glutMotionFunc(motion);

    const char* input_filename = (argc < 2) ? "models/cone.off" : argv[1];
    for (int i = 2; i < argc; ++i)
        printf("ignoring argument: %s\n", argv[++i]);

    initializeShape(input_filename);

    initGL();

#ifdef WIN32
    wglSwapIntervalEXT(0);
#endif

    initHUD();

    glutIdleFunc(idle);
    glutMainLoop();

    quit();
}

//------------------------------------------------------------------------------
