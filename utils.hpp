#pragma once

#include <GL/glew.h>
#include <vector>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "tiny_obj_loader.h"


void get_vertex_array(
        unsigned int* vertex_indexes,
        unsigned int* normals_indexes,
        unsigned int* tex_indexes,
        tinyobj::shape_t shape);