#if defined(__APPLE__)
    #include <GL/glew.h>
    #include <GLUT/glut.h>
#else
    #include <GL/glew.h>
    #if defined(WIN32)
        #include <GL/wglew.h>
    #endif
    #include <GL/glut.h>
#endif
