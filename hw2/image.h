#ifndef _TRACE_IMAGE_H_
#define _TRACE_IMAGE_H_

#include <glm/glm.hpp>

int writeImage(char* filename, int width, int height, glm::vec3 *buffer, char* title);

#endif /* _TRACE_IMAGE_H_ */
