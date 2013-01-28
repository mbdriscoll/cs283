#include "viewer.h"

#include <stdlib.h>
#include <cfloat>
#include <vector>
#include <fstream>
#include <sstream>

#include "Model.h"
#include "extra/gl_hud.h"

Model* g_model = NULL;

int   g_frame = 0,
      g_repeatCount = 0;

// GUI variables
int   g_fullscreen=0,
      g_freeze = 0,
      g_wire = 2,
      g_adaptive = 1,
      g_drawCageEdges = 1,
      g_drawCageVertices = 0,
      g_drawPatchCVs = 0,
      g_drawNormals = 0,
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

GLhud g_hud;

// geometry
std::vector<float> g_orgPositions,
                   g_positions,
                   g_normals;

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
initializeShape(const char* input_filename) {
    printf("Reading object in file %s.\n", input_filename);

    FILE* input_file = fopen(input_filename, "r");
    if (input_file == NULL) {
        printf("Could not open model at %s.\n", input_filename);
        exit(2);
    }

    g_model = new Model(input_file);
    g_model->SetCenterSize((float*) &g_center, &g_size);

    fclose(input_file);
}

//------------------------------------------------------------------------------
static void
updateGeom() {
    g_model->Render();
}

//------------------------------------------------------------------------------
static void
fitFrame() {

    g_pan[0] = g_pan[1] = 0;
    g_dolly = g_size;
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
    glRotatef(-90, 1, 0, 0); // z-up model

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    //glBindBuffer(GL_ARRAY_BUFFER, g_mesh->BindVertexBuffer());

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof (GLfloat) * 6, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof (GLfloat) * 6, (float*)12);

    // drawing
    //glDisableVertexAttribArray(0);
    //glDisableVertexAttribArray(1);

    //if (g_drawNormals)
        //drawNormals();

    g_model->Render();

    //glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    //glDisableClientState(GL_VERTEX_ARRAY);
    glFinish();

    if (g_hud.IsVisible()) {
        g_hud.DrawString(10, -20,  "Tess level : %d", 1337);
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
    }
}

//------------------------------------------------------------------------------
static void
callbackWireframe(int b)
{
    g_wire = b;
}

static void
callbackDisplayNormal(bool checked, int n)
{
    g_drawNormals = checked;
}

static void
callbackFreeze(bool checked, int f)
{
    g_freeze = checked;
}

static void
initHUD()
{
    g_hud.Init(g_width, g_height);

    g_hud.AddCheckBox("Show normal vector (E)", false, 10, 10, callbackDisplayNormal, 0, 'e');
    g_hud.AddCheckBox("Freeze (spc)", false, 10, 30, callbackFreeze, 0, ' ');
}

//------------------------------------------------------------------------------
static void
initGL()
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    //glGenQueries(1, &g_primQuery);
}

//------------------------------------------------------------------------------
static void
idle() {

    if (not g_freeze)
        g_frame++;

    updateGeom();
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

    glewInit();

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
