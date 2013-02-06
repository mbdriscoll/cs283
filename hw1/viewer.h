#if defined(__APPLE__)
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
#endif

#include "glm/glm.hpp"

#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#define N_FRAMES_PER_SPLIT 500
