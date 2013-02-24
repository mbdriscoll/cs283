#include "scene.h"

#include <cstdio>
#include <cstdlib>


Scene::Scene(char *scenefilename) {
    FILE* sfile = fopen(scenefilename, "r");
    if (sfile == NULL) {
        fprintf(stderr, "Unable to open scene file: %s\n", scenefilename);
        exit(2);
    }

    fclose(sfile);
}
